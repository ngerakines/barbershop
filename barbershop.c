/*
Copyright (c) 2010 Nick Gerakines <nick at gerakines dot net>
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* For inet_ntoa. */
#include <arpa/inet.h>

/* Required by event.h. */
#include <sys/time.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>

#include "hashmap.c"
#include <event.h>

/* Port to listen on. */
#define SERVER_PORT    8002

#define KEY_MAX_LENGTH (256)
#define KEY_PREFIX ("somekey")
#define KEY_COUNT (1024*1024)

typedef struct item_s {
    char key_string[KEY_MAX_LENGTH];
    int value;
} item_t;

map_t items;

struct client {
    struct event ev_read;
};

int setnonblock(int fd) {
    int flags;
    flags = fcntl(fd, F_GETFL);
    if (flags < 0) { return flags; }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) { return -1; }
    return 0;
}

typedef struct token_s {
    char *value;
    size_t length;
} token_t;

#define COMMAND_TOKEN 0
#define SUBCOMMAND_TOKEN 1
#define KEY_TOKEN 1
#define VALUE_TOKEN 2
#define MAX_TOKENS 8

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
    int len, wlen;
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

        char key_string[KEY_MAX_LENGTH];
        item_t* item;
        snprintf(key_string, KEY_MAX_LENGTH, "%s%d", KEY_PREFIX, item_id);
        int lookup = hashmap_get(items, key_string, (void**)(&item));
        if (lookup == -3) {
            item = malloc(sizeof(item_t));
            snprintf(item->key_string, KEY_MAX_LENGTH, "%s%d", KEY_PREFIX, item_id);
            item->value = score;
            hashmap_put(items, item->key_string, item);
        } else if (lookup == 0) {
            hashmap_remove(items, item->key_string);
            item->value += score;
            hashmap_put(items, item->key_string, item);
        }
        printf("(%d) item key %s, value %d\n", lookup, item->key_string, item->value);

        printf("Creating/Incrementing record of %d by/for %d.\n", item_id, score);
    } else if (ntokens == 2 && strcmp(tokens[COMMAND_TOKEN].value, "next") == 0) {
        printf("Client wants next item to process.\n");
    } else if (ntokens == 2 && strcmp(tokens[COMMAND_TOKEN].value, "stats") == 0) {
        printf("Client wants server stats.\n");
    } else {
        printf("Fuck'em.\n");
    }
    /* --- end --- */
    wlen = write(fd, buf, len);
    if (wlen < len) {
        printf("Short write, not all data echoed back to client.\n");
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
    items = hashmap_new();
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
