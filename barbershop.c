/*
Copyright (c) 2010 Nick Gerakines <nick at gerakines dot net>
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>

#include "scores.h"
#include "bst.h"
#include "barbershop.h"
#include <event.h>

static size_t tokenize_command(char *command, token_t *tokens, const size_t max_tokens) {
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
        }
        else if (*e == '\0') {
            if (s != e) {
                tokens[ntokens].value = s;
                tokens[ntokens].length = e - s;
                ntokens++;
            }
            break; /* string end */
        }
    }
    tokens[ntokens].value =  *e == '\0' ? NULL : e;
    tokens[ntokens].length = 0;
    ntokens++;
    return ntokens;
}

void on_read(int fd, short ev, void *arg) {
    struct client *client = (struct client *)arg;
    char buf[8196];
    int len;
    len = read(fd, buf, sizeof(buf));
    if (len == 0) {
        printf("Client disconnected.\n");
        close(fd);
        event_del(&client->ev_read);
        free(client);
        return;
    } else if (len < 0) {
        printf("Socket failure, disconnecting client: %s", strerror(errno));
        close(fd);
        event_del(&client->ev_read);
        free(client);
        return;
    }
    /* This could probably be done better. */
    char* nl;
    nl = strrchr(buf, '\r');
    if (nl) { *nl = '\0'; }
    nl = strrchr(buf, '\n');
    if (nl) { *nl = '\0'; }
    /* Figure out what they want to do. */
    token_t tokens[MAX_TOKENS];
    size_t ntokens;
    ntokens = tokenize_command((char*)buf, tokens, MAX_TOKENS);
    if (ntokens == 4 && strcmp(tokens[COMMAND_TOKEN].value, "update") == 0) {
        int item_id = atoi(tokens[KEY_TOKEN].value);
        int score = atoi(tokens[VALUE_TOKEN].value);

        Position lookup = Find( item_id, items );
        if (lookup == NULL) {
            items = Insert(item_id, score, items);
            scores = AddScoreToPool(scores, score, item_id);
        } else {
            int old_score = lookup->score;
            lookup->score += score;
            scores = PurgeThenAddScoreToPool(scores, lookup->score, item_id, old_score);
        }
        reply(fd, "OK\n");
    } else if (ntokens == 2 && strcmp(tokens[COMMAND_TOKEN].value, "next") == 0) {
        reply(fd, "OK\n");
    } else if (ntokens == 2 && strcmp(tokens[COMMAND_TOKEN].value, "stats") == 0) {
        printf("Dumping items tree:\n");
        DumpItems(items);
        printf("Dumping score buckets:\n");
        DumpScores(scores);
        printf("Client wants server stats.\n");
        reply(fd, "OK\n");
    } else {
        reply(fd, "NOOP\n");
    }
}

void on_accept(int fd, short ev, void *arg) {
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct client *client;
    client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1) {
        warn("accept failed");
        return;
    }
    if (setnonblock(client_fd) < 0) {
        warn("failed to set client socket non-blocking");
    }
    client = calloc(1, sizeof(*client));
    if (client == NULL) {
        err(1, "malloc failed");
    }
    event_set(&client->ev_read, client_fd, EV_READ|EV_PERSIST, on_read, client);
    event_add(&client->ev_read, NULL);
    printf("Accepted connection from %s\n", inet_ntoa(client_addr.sin_addr));
}

int main(int argc, char **argv) {
    items = MakeEmpty(NULL);
    scores = PrepScoreBucket(NULL);
    int listen_fd;
    struct sockaddr_in listen_addr;
    int reuseaddr_on = 1;
    struct event ev_accept;
    event_init();
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { err(1, "listen failed"); }
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on)) == -1) { err(1, "setsockopt failed"); }
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(SERVER_PORT);
    if (bind(listen_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) { err(1, "bind failed"); }
    if (listen(listen_fd, 5) < 0) { err(1, "listen failed"); }
    if (setnonblock(listen_fd) < 0) { err(1, "failed to set server socket to non-blocking"); }
    event_set(&ev_accept, listen_fd, EV_READ|EV_PERSIST, on_accept, NULL);
    event_add(&ev_accept, NULL);
    event_dispatch();
    return 0;
}

int setnonblock(int fd) {
    int flags;
    flags = fcntl(fd, F_GETFL);
    if (flags < 0) { return flags; }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) { return -1; }
    return 0;
}

void reply(int fd, char *buffer) {
    int n = write(fd, buffer, strlen(buffer));
    if (n < 0 || n < strlen(buffer))
         printf("ERROR writing to socket");
}
