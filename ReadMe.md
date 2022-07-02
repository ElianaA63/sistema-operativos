# SISTEMAS OPERATIVOS 2021 - LABORATORIO 1: MyBash

## Grupo 5:
 * Arias, Eliana - eliana.arias@mi.unc.edu.ar
 * Leiva, Mauro - mauro.leiva@mi.unc.edu.ar
 * Toyos, Milagros - milagros.toyos@mi.unc.edu.ar

## Modo de trabajo
Cada uno de los integrantes del grupo colaboró con el trabajo usando su computadora y programando
en tiempo real conectados por Google meet/grupo en Whatsapp.

## Modo de uso
Mover los archivos parser.o y lexer.o al directorio principal. Elegir la opción adecuada según la arquitectura del procesador empleado (utilizar los archivos del directorio objects-x86_64 para procesadores de 64 bits, o los del directorio objects-i386 para procesadores de 32 bits).
Compilar utilizando el comando
$ make
Ejecutar MyBash utilizando el comando
$ ./mybash

Pueden ejecutarse los tests provistos por la cátedra con los siguientes comandos (desde el directorio principal):
$ make test-command  //  Corre tests del archivo command.c
$ make test // Corre tests de los archivos command.c y execute.c
$ make memtest // Corre tests de manejo de memoria del programa

El bash compilado permite la ejecución de comandos mediante un pipeline (de hasta dos comandos simples).
Algunos comandos que pueden utilizarse como prueba de MyBash:
mybash> ls -l
mybash> wc -l > out.txt < in.txt
mybash> ls | wc -l
mybash> /usr/bin/xeyes &
mybash> cd 'directorio'
mybash> exit

El bash permite la ejecución de comandos tanto en modo foreground como en modo background.

## Modularización
Se mantuvo la división en cuatro módulos (command, parser, builtin y execute) propuesta por la cátedra.
Dentro de 'execute' se agregaron tres algoritmos auxiliares: 'execute_scommand', 'redir_in' y 'redir_out'. Estas funciones ayudan a modularizar el trabajo dentro del algoritmo principal 'execute_pipeline', permitiendo evitar la repetición innecesaria de código y facilitar la lectura del mismo.
En los módulos restantes, las funciones fueron implementadas de acuerdo a sus respectivas especificaciones.

## Técnicas de Programación
Se utilizó la librería "glib.h", que provee diferentes implementaciones de listas; en particular, la empleada en el laboratorio fue 'GSList' (listas simples enlazadas).
