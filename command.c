#include <stdio.h>
#include <glib.h>
#include <assert.h>
#include <stdlib.h>

#include "strextra.h"
#include "command.h"
#include "tests/syscall_mock.h"


/**** COMANDO SIMPLE ****/

/* Estructura correspondiente a un comando simple.
 * Es una 3-upla del tipo ([char*], char* , char*).
 */

struct scommand_s {
    GSList *args;
    char * redir_in;
    char * redir_out;
};


scommand scommand_new(void){
	scommand s = NULL;
	s = malloc(sizeof(struct scommand_s));
	s->args = NULL;
	s->redir_in = NULL;
	s->redir_out = NULL;
	assert(s != NULL && scommand_is_empty (s)
	&& scommand_get_redir_in (s) == NULL
	&& scommand_get_redir_out (s) == NULL);
    return s;
}

scommand scommand_destroy(scommand self){
    assert(self != NULL);
    if(self->args != NULL){
		g_slist_free_full(self->args,free);
		self->args = NULL;
	}
	if (self->redir_in != NULL){
		free(self ->redir_in);
		self->redir_in = NULL;
	}
	if (self->redir_out != NULL){
		free(self ->redir_out);
		self->redir_out = NULL;
	}
	free(self);
	self = NULL;
	return self; 
}

void scommand_push_back(scommand self, char * argument){
	assert(self != NULL && argument != NULL);
    self->args = g_slist_append(self->args, argument);
	assert(!scommand_is_empty(self));
}

void scommand_pop_front(scommand self){
	assert(self != NULL && !scommand_is_empty(self));
    if(self->args != NULL){
        char *aux = g_slist_nth_data(self->args, 0);
        self->args = g_slist_remove(self->args, aux);
        free(aux);
    }
}

void scommand_set_redir_in(scommand self, char * filename){
	assert(self != NULL);
    if(self->redir_in != NULL){
        free(self->redir_in);
    }
    self->redir_in = filename;
}

void scommand_set_redir_out(scommand self, char * filename){
	assert(self != NULL);
    if(self->redir_out != NULL){
        free(self->redir_out);
    }
    self->redir_out = filename;
}

bool scommand_is_empty(const scommand self){
	assert(self != NULL);
	return scommand_length(self) == 0;
}

unsigned int scommand_length(const scommand self){
	assert(self != NULL);
	unsigned int n = 0u;
	if(self->args != NULL){
		n = g_slist_length(self->args);
	}
	return n;
}

char * scommand_front(const scommand self){
	assert(self !=NULL && !scommand_is_empty(self));
	char * aux = g_slist_nth_data(self->args, 0);
	return aux;
}

char * scommand_get_redir_in(const scommand self){
	assert(self != NULL);
    return self->redir_in;
}

char * scommand_get_redir_out(const scommand self){
	assert(self != NULL);
    return self->redir_out;
}

static char * strmerge_f(char *s1, char *s2){
  char *result = strmerge(s1, s2);
  free(s1);
  return result;
}

char *scommand_to_string(const scommand self){
    assert(self != NULL);
    char *result = strdup("");
	unsigned int n = scommand_length(self);
	for(unsigned int i = 0u; i < n; i++){
	result = strmerge_f(result, g_slist_nth_data(self->args, i));
		if(i < n-1){
			result = strmerge_f(result, " ");
		}
	}
	if(self->redir_in != NULL){
	result = strmerge_f(result, "< ");
	result = strmerge_f(result, scommand_get_redir_in(self));
	result = strmerge_f(result, " ");
	}
	if(self->redir_out != NULL){
	result = strmerge_f(result, "> ");
	result = strmerge_f(result, scommand_get_redir_out(self));
	}
    return result;
}


/********** COMANDO PIPELINE **********/

/* Estructura correspondiente a un comando pipeline.
 * Es un 2-upla del tipo ([scommand], bool)
 */

struct pipeline_s {
    GSList *scmds;
    bool wait;
};


pipeline pipeline_new(void){
	pipeline pipe = malloc(sizeof(struct pipeline_s));
	pipe->scmds = NULL;
	pipe->wait = true;
	assert(pipe != NULL && pipeline_is_empty(pipe) && pipeline_get_wait(pipe));
    return pipe;
}

pipeline pipeline_destroy(pipeline self){
    assert(self!= NULL);
    while(self->scmds){
        self->scmds->data = scommand_destroy(self->scmds->data);
        self->scmds = g_slist_delete_link(self->scmds,self->scmds);
    }
    free(self);
    self = NULL;
    return self;
}

void pipeline_push_back(pipeline self, scommand sc){
	assert(self != NULL && sc != NULL);
    self->scmds = g_slist_append(self->scmds, sc);
}

void pipeline_pop_front(pipeline self){
	assert(self != NULL && !pipeline_is_empty(self));
    if(self->scmds != NULL){
        char *aux = g_slist_nth_data(self->scmds, 0);
        self->scmds = g_slist_remove(self->scmds, aux);
        free(aux);
    }
}

void pipeline_set_wait(pipeline self, const bool w){
	assert(self != NULL);
	self->wait = w;
}

bool pipeline_is_empty(const pipeline self){
	assert(self != NULL);
	return pipeline_length(self) == 0;
}

unsigned int pipeline_length(const pipeline self){
	assert(self != NULL);
	unsigned int n = 0u;
	if(self->scmds != NULL){
		n = g_slist_length(self->scmds);
	}
	return n;
}

scommand pipeline_front(const pipeline self){
	assert(self != NULL && !pipeline_is_empty(self));
	scommand aux = g_slist_nth_data(self->scmds, 0);
	return aux;
}

bool pipeline_get_wait(const pipeline self){
	assert(self != NULL);
	return self->wait;
}

char * pipeline_to_string(const pipeline self){
	assert(self != NULL);
	char *result = strdup("");
	if(self->scmds != NULL){
	unsigned int n = g_slist_length(self->scmds);
	char *aux = NULL;
	for(unsigned int i = 0u; i < n; i++){
		aux = scommand_to_string(g_slist_nth_data(self->scmds, i));
		result = strmerge_f(result, aux);
		free(aux);
		aux = NULL;
		if(i < n-1){
		result = strmerge_f(result, "| ");
		}
	}
		if(!self->wait){
			result = strmerge_f(result, " & ");
		}
	}
	return result;
}