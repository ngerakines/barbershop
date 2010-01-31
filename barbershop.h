
#include <event.h>

#define COMMAND_TOKEN 0
#define SUBCOMMAND_TOKEN 1
#define KEY_TOKEN 1
#define VALUE_TOKEN 2
#define MAX_TOKENS 8
#define SERVER_PORT    8002

struct client {
    struct event ev_read;
};

struct TreeNode {
    int item;
    int score;
    SearchTree  left;
    SearchTree  right;
};

struct member_el {
    int item;
    MemberBucket next;
};

struct bucket_el {
    int score;
    int count;
    MemberBucket members;
    ScoreBucket next;
};

typedef struct token_s {
    char *value;
    size_t length;
} token_t;

SearchTree items;
ScoreBucket scores;

static size_t tokenize_command(char *command, token_t *tokens, const size_t max_tokens);
void on_read(int fd, short ev, void *arg);
void on_accept(int fd, short ev, void *arg);
int main(int argc, char **argv);
int setnonblock(int fd);
void reply(int fd, char *buffer);

