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

// XXX: Unused, to be removed.
PoolNode *pool_insert_after(PoolNode *node, int score) {
	PoolNode *newnode;
	newnode = pool_create(score);
	newnode->next = node->next;
	node->next = newnode;
	return newnode;
}

PoolNode *pool_push(PoolNode *list, int score) {
	PoolNode *newnode;
	newnode = pool_create(score);
	newnode->next = list;
	return newnode;
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

int pool_foreach(PoolNode *node, int(*func)(int, MemberNode*)) {
	while (node) {
		if (func(node->score, node->members) != 0) {
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
	// printf("find_by_score(%d, NULL, %d)\n", score, query);
	return score == query;
}

int find_item(int item, void *query) {
	// printf("find_item(%d, %d)\n", item, query);
	return item == query;
}

int pool_print(int score, MemberNode *members) {
	printf("Pool %d\n", score);
	member_foreach(members, member_print);
	return 0;
}

int member_print(int item) {
	printf(" -> item %d\n", item);
	return 0;
}

int preparePromotion(PoolNode *list, int item, int score) {
	// printf("preparePromotion called.\n");
	PoolNode *listMatch;
	if ((listMatch = pool_find(list, find_by_score, (void*)score))) {
		// printf(" => found pool with score %d\n", score);
		MemberNode *memberMatch;
		if ((memberMatch = member_find(listMatch->members, find_item, (void*)item))) {
			// printf(" => found member %d in pool %d\n", item, score);
			listMatch->count--;
			member_remove(listMatch->members, memberMatch);
			return 1;
		}
	}
	return 0;
}

PoolNode *promoteItem(PoolNode *list, int score, int item, int old_score) {
	if (old_score != -1) {
		preparePromotion(list, item, old_score);
	}
	// printf("promoteItem called.\n");
	if (list == NULL) {
		// printf(" -> list is NULL\n");
		PoolNode *newPool;
		newPool = pool_create(score);
		// printf(" -> Created newPool (score=%d)\n", newPool->score);
		MemberNode *newMember;
		newMember = member_create(item);
		// printf(" -> Created newMember (item=%d)\n", newMember->item);
		newPool->members = newMember;
		newPool->count++;
		return newPool;
	}
	PoolNode *listMatch;
	// printf(" -> looking for score %d.\n", score);
	if ((listMatch = pool_find(list, find_by_score, (void*)score))) {
		// printf(" -> score pool exists (score=%d)\n", listMatch->score);
		MemberNode *memberMatch;
		if ((memberMatch = member_find(listMatch->members, find_item, (void*)item))) {
			// printf(" -> item exists in pool\n");
		} else {
			// printf(" -> item does not exist in pool\n");
			assert(listMatch != NULL);
			assert(listMatch->members != NULL);
			listMatch->members = member_push(listMatch->members, item);
			// printf(" -> new member created (item=%d)\n", listMatch->members->item);
			listMatch->count++;
			// printf(" -> pool %d has %d members\n", listMatch->score, listMatch->count);
		}
		return list;
	} else {
		// Score pool doesn't exist
		// printf(" -> score pool doesn't exist\n");
		PoolNode *newPool;
		newPool = pool_create(score);
		// printf(" -> Created newPool (score=%d)\n", newPool->score);
		MemberNode *newMember;
		newMember = member_create(item);
		// printf(" -> Created newMember (item=%d)\n", newMember->item);
		newPool->members = newMember;
		newPool->count++;
		newPool->next = list;
		return newPool;
	}
	return NULL;
}

// int NextItem(PoolNode *list) {
// 	if (! list) { return -1; }
// 	while (list->next && list->count > 0) {
// 		list = list->next;
// 	}
// 	if (list != NULL && list->members != NULL) {
// 		MemberNode *last = member_last(list->members);
// 		int item_id = last->item;
// 		list->count--;
// 		if (list->members == 1) {
// 			free(last);
// 			list->members = NULL;
// 		} else {
// 			member_remove(list->members, last);
// 		}
// 		return item_id;
// 	} else{
// 		return -1;
// 	}
// }

PoolNode *NextItem(PoolNode *list, int *next_item) {
	if (list == NULL) { next_item = -1; return list; }
	while (list->next && list->count == 0) {
		list = list->next;
	}
	if (list != NULL && list->members != NULL) {
		printf("We got this far ...\n");
		MemberNode *last = member_last(list->members);
		next_item = last->item;
		printf("and now ...\n");
		member_remove(list->members, next_item);
		printf("and now ...\n");
		if (list->members != NULL) { printf("first member item is %d\n", list->members->item); }
		printf("count at %d\n", list->count);
		list->count--;
		return list;
	}
	return list;
}

