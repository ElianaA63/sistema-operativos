#include "builtin.h"
#include "command.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "tests/syscall_mock.h"

bool builtin_is_exit(pipeline pipe){
	assert(pipe != NULL);
	bool result = !strcmp(pipeline_to_string(pipe), "exit");
	return result;
}

bool builtin_is_cd(pipeline pipe){
	assert(pipe != NULL);
	bool result = !strncmp(pipeline_to_string(pipe), "cd", 2);
	return result;
}

bool builtin_is_internal(pipeline pipe){
	assert(pipe != NULL);
	return builtin_is_exit(pipe) || builtin_is_cd(pipe);
}

void builtin_exec(pipeline pipe){
	assert(builtin_is_internal(pipe));
	scommand aux = pipeline_front(pipe); 
	if (builtin_is_cd(pipe)){
		scommand_pop_front(aux);
		char *directory = scommand_to_string(aux);
		if (chdir(directory) != 0){
			fprintf(stderr, "Error accediendo al directorio\n");
		}
		free(directory);
	}
	if (builtin_is_exit(pipe)){
		exit(EXIT_SUCCESS);		
	}
}