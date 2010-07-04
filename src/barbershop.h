/*
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

#ifndef __BARBERSHOP_H__
#define __BARBERSHOP_H__

#include <event.h>

#define SERVER_PORT			8002
#define RUNNING_DIR			"/tmp"
#define LOCK_FILE			"barbershop.lock"
#define LOG_FILE			"barbershop.log"

struct client {
	struct event ev_read;
};

pthread_mutex_t scores_mutex;
int timeout;
char *sync_file;

int main(int argc, char **argv);

void on_read(int fd, short ev, void *arg);
void on_accept(int fd, short ev, void *arg);
int setnonblock(int fd);
void gc_thread();
void load_snapshot(char *filename);
void sync_to_disk(char *filename);

void daemonize();
void signal_handler(int sig);

#endif
