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

#include "scores.h"
#include "stats.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

PoolNode *pool_create(int score) {
	PoolNode *node;
	if (! (node = malloc(sizeof(PoolNode)))) {
		return NULL;
	}
	node->score = score;
	node->count = 0;
	node->members = NULL;
	node->next = NULL;
	return node;
}

int pool_remove(PoolNode *list, PoolNode *node) {
	while (list->next && list->next != node) {
		list = list->next;
	}
	if (list->next) {
		list->next = node->next;
		free(node);
		return 0;
	} else {
		return -1;
	}
}

int pool_foreach(PoolNode *node, int(*func)(int, int, MemberNode*)) {
	while (node) {
		if (func(node->score, node->count, node->members) != 0) {
			return -1;
		}
		node = node->next;
	}
	return 0;
}

PoolNode *pool_find(PoolNode *node, int(*func)(int, MemberNode*,void*), void *data) {
	while (node) {
		if (func(node->score, node->members, data) > 0) {
			return node;
		}
		node = node->next;
	}
	return NULL;
}

MemberNode *member_create(int item) {
	MemberNode *node;
	if (! (node = malloc(sizeof(MemberNode)))) {
		return NULL;
	}
	node->item = item;
	node->next = NULL;
	return node;
}

MemberNode *member_push(MemberNode *list, int item) {
	MemberNode *newnode;
	newnode = member_create(item);
	newnode->next = list;
	return newnode;
}

int member_remove(MemberNode *list, MemberNode *node) {
	while (list->next && list->next != node) {
		list = list->next;
	}
	if (list->next) {
		list->next = node->next;
		free(node);
		return 0;
	} else {
		return -1;
	}
}

int member_foreach(MemberNode *node, int(*func)(int)) {
	while (node) {
		if (func(node->item) != 0) {
			return -1;
		}
		node = node->next;
	}
	return 0;
}

MemberNode *member_find(MemberNode *node, int(*func)(int, void*), void *data) {
	while (node) {
		if (func(node->item, data) > 0) {
			return node;
		}
		node = node->next;
	}
	return NULL;
}

MemberNode *member_last(MemberNode *node) {
	while (node) {
		if (node->next == NULL) {
			return node;
		}
		node = node->next;
	}
	return NULL;
}

int find_by_score(int score, MemberNode *members, void *query) {
	return score == query;
}

int find_item(int item, void *query) {
	return item == query;
}

int pool_print(int score, int count, MemberNode *members) {
	printf("Pool %d (count %d)\n", score, count);
	member_foreach(members, member_print);
	return 0;
}

int member_print(int item) {
	printf(" -> item %d\n", item);
	return 0;
}

PoolNode *preparePromotion(PoolNode *head, int item, int score) {
	MemberNode *memberMatch;
	if (head->score == score) {
		if (head->members->item == item) {
			memberMatch = head->members;
			head->members = memberMatch->next;
			head->count -= 1;
		} else {
			if ((memberMatch = member_find(head->members, find_item, (void*)item))) {
				head->count -= 1;
				assert(member_remove(head->members, memberMatch) == 0);
			}
		}
		if (head->count == 0) {
			return head->next;
		}
	} else {
		PoolNode *listMatch;
		if ((listMatch = pool_find(head, find_by_score, (void*)score))) {
			if (listMatch->count == 1) {
				assert(pool_remove(head, listMatch) == 0);
				return 1;
			} else {
				if ((memberMatch = member_find(listMatch->members, find_item, (void*)item))) {
					listMatch->count -= 1;
					assert(member_remove(listMatch->members, memberMatch) == 0);
					return 1;
				}
				return 0;
			}
		}
	}
	return head;
}

PoolNode *promoteItem(PoolNode *list, int score, int item, int old_score) {
	if (old_score != -1) {
		list = preparePromotion(list, item, old_score);
	}
	if (list == NULL) {
		PoolNode *newPool;
		newPool = pool_create(score);
		MemberNode *newMember;
		newMember = member_create(item);
		newPool->members = newMember;
		newPool->count++;
		return newPool;
	}
	PoolNode *listMatch;
	if ((listMatch = pool_find(list, find_by_score, (void*)score))) {
		MemberNode *memberMatch;
		if ((memberMatch = member_find(listMatch->members, find_item, (void*)item))) {
		} else {
			assert(listMatch != NULL);
			assert(listMatch->members != NULL);
			listMatch->members = member_push(listMatch->members, item);
			listMatch->count++;
		}
		return list;
	} else {
		// Score pool doesn't exist
		PoolNode *newPool;
		newPool = pool_create(score);
		MemberNode *newMember;
		newMember = member_create(item);
		newPool->members = newMember;
		newPool->count++;
		newPool->next = list;
		return newPool;
	}
	return NULL;
}

PoolNode *NextItem(PoolNode *head, int *next_item) {
	if (head == NULL) {
		*next_item = -1;
		return NULL;
	}
	assert(head != NULL);
	if (head->count == 1)
	{
		// If the head has one member then set the next_item to the
		// members->item value and return head->next
		*next_item = head->members->item;
		return head->next;
	}
	else
	{
		// If the head has more than one member then
		// * get the value of the last members->item
		// * remove the last member of the head->members chain
		// * return head
		MemberNode *last = member_last(head->members);
		*next_item = last->item;
		assert(member_remove(head->members, last) == 0);
		head->count -= 1;
		return head;
	}
}

