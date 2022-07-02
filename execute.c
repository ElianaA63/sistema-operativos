#include "command.h"
#include "builtin.h"
#include "execute.h"
#include <assert.h>
#include <stddef.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>


#include "tests/syscall_mock.h"

/* Función auxiliar para ejecutar un comando simple */
static void execute_scommand(scommand command){
    unsigned int scmd_len = scommand_length(command);

    /* Crear un arreglo que contendrá el comando y sus argumentos */
    char** scmd = malloc((scmd_len + 1) * sizeof(char*));
    for (unsigned int i = 0u; i < scmd_len; i++){
        char *aux = scommand_front(command);
        scmd[i] = strdup(aux);
        scommand_pop_front(command);
    }

    /* Asignar NULL a la última posición del arreglo
     para el correcto funcionamiento de execvp */
    scmd[scmd_len] = NULL;

    /* Ejecutar el comando */
    if(execvp(scmd[0], scmd) != 0){
        perror("Error ejecutando el comando");
    }

    /* Liberar la memoria pedida por malloc */
    for (unsigned int i = 0u; i < scmd_len; i++){
        free(scmd[i]);
    }
    free(scmd);
}

/* Función auxiliar para redirigir entrada */
static void redir_in(scommand scmd){
    assert(scmd != NULL);

    /* Obtener el nombre de archivo a donde realizar la redirección */
    char *redir = scommand_get_redir_in(scmd);
    int new_in;

    /* En caso que la redirección no sea NULL,
     abrir el archivo requerido y copiar Standard Input */
    if(redir){
        new_in = open(redir, O_RDONLY, S_IRWXU);
        dup2(new_in, STDIN_FILENO);
        close(new_in);
    }
}

/* Función auxiliar para redirigir salida */
static void redir_out(scommand scmd){
    assert(scmd != NULL);

    /* Obtener el nombre de archivo a donde realizar la redirección */
    char *redir = scommand_get_redir_out(scmd);
    int new_out;

    /* En caso que la redirección no sea NULL,
     abrir el archivo requerido y copiar Standard Output */
    if(redir){
        new_out = open(redir, O_CREAT | O_WRONLY, S_IRWXU);
        dup2(new_out, STDOUT_FILENO);
        close(new_out);
    }
}

void execute_pipeline(pipeline apipe){
    assert(apipe != NULL);

    /* Verificar si el pipeline es vacío */
    if(pipeline_is_empty(apipe)){
        return;
    }
    
    /* Verificar si el pipeline es un comando interno; ejecutarlo de ser así */
    if(builtin_is_internal(apipe)){
        builtin_exec(apipe);
        return;
    }

    /* Si el pipeline contiene un solo comando simple, ejecutarlo sin pipes */
    if(pipeline_length(apipe) == 1){
        pid_t process = fork();

        /* Si la id del proceso es 0, el proceso es el hijo creado por fork */
        if(process == 0){ 
            /* Redireccionar entrada y salida, en caso de ser necesario */
            redir_in(pipeline_front(apipe));
            redir_out(pipeline_front(apipe));

            /* Ejecutar el comando externo */
            execute_scommand(pipeline_front(apipe));
        }
        /* Si la id del proceso NO es 0, el proceso es el padre */
        else{
            if(pipeline_get_wait(apipe)){
                waitpid(process, NULL, 0);
            }
        }
        return;

    /* Si el pipeline contiene dos comandos, ejecutarlo con un pipe */
    } else {

        /* Crear un arreglo de file descriptors (read and write) */
        int fd_pipe[2];

        /* Systemcall Pipe; si el valor devuelto es -1, falló la ejecución */
        if(pipe(fd_pipe) == -1){
            perror("PIPE FAILED");
        }
        
        /* Variables auxiliares */
        bool pipe_wait = pipeline_get_wait(apipe);
        pid_t pid1;
        pid_t pid2;

        /* Ejecución del primer fork */
        pid1 = fork();

        /* Si la id del fork es -1, falló la ejecución */
        if(pid1 < 0){
            perror("FORK FAILED");
        }

        /* Si la id del fork es 0, estoy en el hijo creado */
        if(pid1 == 0){
            /* Redireccionar la salida del comando, en caso de necesaria */
            redir_out(pipeline_front(apipe));

            /* Dirigir salida al pipe y cerrar ambas puntas */
            close(fd_pipe[0]);
            dup2(fd_pipe[1], STDOUT_FILENO);
            close(fd_pipe[1]);

            /* Ejecutar el comando */
            execute_scommand(pipeline_front(apipe));

        } else { // Si la id del fork es mayor que 0, estoy en el padre

            /* Ejecución del segundo fork */
            pid2 = fork();

            /* Si la id del fork es -1, falló la ejecución */
            if(pid2 < 0){
            perror("FORK FAILED");
            }

            /* Si la id del fork es 0, estoy en el segundo hijo creado */
            if(pid2 == 0){
                /* Remover el primer comando del pipe 
                (a ser ejecutado por el primer hijo) */
                pipeline_pop_front(apipe);

                /* Redireccinar la entrada del comando, de ser necesario */
                redir_in(pipeline_front(apipe));

                /* Dirigir entrada al pipe y cerrar ambas puntas */
                close(fd_pipe[1]);
                dup2(fd_pipe[0], STDIN_FILENO);
                close(fd_pipe[0]);

                /* Ejecutar el comando */
                execute_scommand(pipeline_front(apipe));

            } else { // Estoy en el padre luego de ambos forks

                /* Cerrar ambas puntas */
                close(fd_pipe[0]);
                close(fd_pipe[1]);

                /* Si el pipeline recibido tenía la señal de espera, 
                esperar a los hijos creados en forks */
                if(pipe_wait){
                    waitpid(pid1, NULL, 0);
                    waitpid(pid2, NULL, 0);
                }
            }
        }
    }
}