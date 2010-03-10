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

#ifndef __SCORES_H__
#define __SCORES_H__

struct node_member;
struct node_pool;
typedef struct node_member *MemberNode;
typedef struct node_pool *PoolNode;

struct node_member {
	int item;
	MemberNode next;
};

struct node_pool {
	int score;
	MemberNode members;
	int count;
	PoolNode next;
};

PoolNode pool_create(int score);
int pool_remove(PoolNode list, PoolNode node);
int pool_foreach(PoolNode node, int(*func)(int, int, MemberNode));
PoolNode pool_find(PoolNode node, int(*func)(int, MemberNode, int), int data);

MemberNode member_create(int item);
MemberNode member_push(MemberNode list, int item);
int member_remove(MemberNode list, MemberNode node);
int member_foreach(MemberNode node, int(*func)(int));
MemberNode member_find(MemberNode node, int(*func)(int, int), int data);
MemberNode member_last(MemberNode node);

int find_by_score(int score, MemberNode members, int query);
int find_item(int item, int query);

PoolNode preparePromotion(PoolNode head, int item, int score);
PoolNode promoteItem(PoolNode list, int score, int item, int old_score);
void PeekNext(PoolNode head, int *next_item);
PoolNode NextItem(PoolNode list, int *next_item);

#ifdef DEBUG
int pool_print(int score, int count, MemberNode members);
int member_print(int item);
#endif

#endif
