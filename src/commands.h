/*
 Modified by Dwayn Matthies <dwayn(dot)matthies(at)gmail(dot)com>
 to use pqueue.h to handle the priority queue
Copyright (c) 2010 Nick Gerakines <nick at gerakines dot net>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#define COMMAND_TOKEN		0
#define SUBCOMMAND_TOKEN	1
#define KEY_TOKEN			1
#define VALUE_TOKEN			2
#define MAX_TOKENS			8

typedef struct token_s {
	char *value;
	size_t length;
} token_t;

void command_update(int fd, token_t *tokens);
void command_next(int fd, token_t *tokens);
void command_peek(int fd, token_t *tokens);
void command_score(int fd, token_t *tokens);
void command_info(int fd, token_t *tokens);
void process_request(int fd, char *input);
size_t tokenize_command(char *command, token_t *tokens, const size_t max_tokens);
void reply(int fd, char *buffer);

#endif
