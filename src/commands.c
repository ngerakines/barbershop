
#include <arpa/inet.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "commands.h"
#include "scores.h"
#include "bst.h"
#include "stats.h"
#include "barbershop.h"

void command_update(int fd, token_t *tokens) {
	int item_id = atoi(tokens[KEY_TOKEN].value);
	int score = atoi(tokens[VALUE_TOKEN].value);
	if (item_id == 0) {
		reply(fd, "-ERROR INVALID ITEM ID\r\n");
		return;
	}
	if (score < 1) {
		reply(fd, "-ERROR INVALID SCORE\r\n");
		return;
	}
	Position lookup = Find(item_id, items);
	if (lookup == NULL) {
		items = Insert(item_id, score, items);
		pthread_mutex_lock(&scores_mutex);
		scores = promoteItem(scores, score, item_id, -1);
		pthread_mutex_unlock(&scores_mutex);
		app_stats.items += 1;
	} else {
		int old_score = lookup->score;
		if (old_score == -1) {
			lookup->score = 1;
		} else {
			lookup->score += score;
		}
		assert(lookup->score > old_score);
		pthread_mutex_lock(&scores_mutex);
		scores = promoteItem(scores, lookup->score, item_id, old_score);
		pthread_mutex_unlock(&scores_mutex);
	}
	app_stats.updates += 1;
	reply(fd, "+OK\r\n");
}

void command_next(int fd, token_t *tokens) {
	int next;
	pthread_mutex_lock(&scores_mutex);
	scores = NextItem(scores, &next);
	pthread_mutex_unlock(&scores_mutex);
	if (next != -1) {
		Position lookup = Find( next, items );
		if (lookup != NULL) {
			lookup->score = -1;
		}
		app_stats.items -= 1;
	}
	char msg[32];
	sprintf(msg, "+%d\r\n", next);
	reply(fd, msg);
	
}

void command_peek(int fd, token_t *tokens) {
	int next;
	pthread_mutex_lock(&scores_mutex);
	PeekNext(scores, &next);
	pthread_mutex_unlock(&scores_mutex);
	char msg[32];
	sprintf(msg, "+%d\r\n", next);
	reply(fd, msg);
}

void command_score(int fd, token_t *tokens) {
	int item_id = atoi(tokens[KEY_TOKEN].value);
	if (item_id == 0) {
		reply(fd, "-ERROR INVALID ITEM ID\r\n");
		return;
	}
	Position lookup = Find(item_id, items);
	if (lookup == NULL) {
		reply(fd, "+-1\r\n");
	} else {
		char msg[32];
		sprintf(msg, "+%d\r\n", lookup->score);
		reply(fd, msg);
	}
}

void command_info(int fd, token_t *tokens) {
	char out[128];
	time_t current_time;
	time(&current_time);
	sprintf(out, "uptime:%d\r\n", (int)(current_time - app_stats.started_at)); reply(fd, out);
	sprintf(out, "version:%s\r\n", app_stats.version); reply(fd, out);
	sprintf(out, "updates:%u\r\n", app_stats.updates); reply(fd, out);
	sprintf(out, "items:%u\r\n", app_stats.items); reply(fd, out);
	sprintf(out, "pools:%u\r\n", app_stats.pools); reply(fd, out);
}

// TODO: Clean the '\r\n' scrub code.
// TODO: Add support for the 'quit' command.
void process_request(int fd, char *input) {
	char* nl;
	nl = strrchr(input, '\r');
	if (nl) { *nl = '\0'; }
	nl = strrchr(input, '\n');
	if (nl) { *nl = '\0'; }
	token_t tokens[MAX_TOKENS];
	size_t ntokens = tokenize_command(input, tokens, MAX_TOKENS);
	if (ntokens == 4 && strcmp(tokens[COMMAND_TOKEN].value, "UPDATE") == 0) {
		command_update(fd, tokens);
	} else if (ntokens == 2 && strcmp(tokens[COMMAND_TOKEN].value, "PEEK") == 0) {
		command_peek(fd, tokens);
	} else if (ntokens == 2 && strcmp(tokens[COMMAND_TOKEN].value, "NEXT") == 0) {
		command_next(fd, tokens);
	} else if (ntokens == 3 && strcmp(tokens[COMMAND_TOKEN].value, "SCORE") == 0) {
		command_score(fd, tokens);
	} else if (ntokens == 2 && strcmp(tokens[COMMAND_TOKEN].value, "INFO") == 0) {
		command_info(fd, tokens);
	} else {
		reply(fd, "-ERROR\r\n");
	}
}

size_t tokenize_command(char *command, token_t *tokens, const size_t max_tokens) {
	char *s, *e;
	size_t ntokens = 0;
	for (s = e = command; ntokens < max_tokens - 1; ++e) {
		if (*e == ' ') {
			if (s != e) {
				tokens[ntokens].value = s;
				tokens[ntokens].length = e - s;
				ntokens++;
				*e = '\0';
			}
			s = e + 1;
		} else if (*e == '\0') {
			if (s != e) {
				tokens[ntokens].value = s;
				tokens[ntokens].length = e - s;
				ntokens++;
			}
			break;
		}
	}
	tokens[ntokens].value =  *e == '\0' ? NULL : e;
	tokens[ntokens].length = 0;
	ntokens++;
	return ntokens;
}

void reply(int fd, char *buffer) {
	int n = write(fd, buffer, strlen(buffer));
	if (n < 0 || n < strlen(buffer)) {
		printf("ERROR writing to socket");
	}
}
