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

PoolNode pool_create(int score) {
	PoolNode node;
	node = malloc( sizeof( struct node_pool ) );
	assert(node != NULL);
	node->score = score;
	node->count = 0;
	node->members = NULL;
	node->next = NULL;
	return node;
}

int pool_remove(PoolNode list, PoolNode node) {
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

int pool_foreach(PoolNode node, int(*func)(int, int, MemberNode)) {
	while (node) {
		if (func(node->score, node->count, node->members) != 0) {
			return -1;
		}
		node = node->next;
	}
	return 0;
}

PoolNode pool_find(PoolNode node, int(*func)(int, MemberNode, int), int data) {
	while (node) {
		if (func(node->score, node->members, data) > 0) {
			return node;
		}
		node = node->next;
	}
	return NULL;
}

MemberNode member_create(int item) {
	MemberNode node;
	node = malloc( sizeof( struct node_member ) );
	assert(node != NULL);
	node->item = item;
	node->next = NULL;
	return node;
}

MemberNode member_push(MemberNode list, int item) {
	MemberNode newnode;
	newnode = member_create(item);
	newnode->next = list;
	return newnode;
}

int member_remove(MemberNode list, MemberNode node) {
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

int member_foreach(MemberNode node, int(*func)(int)) {
	while (node) {
		if (func(node->item) != 0) {
			return -1;
		}
		node = node->next;
	}
	return 0;
}

MemberNode member_find(MemberNode node, int(*func)(int, int), int data) {
	while (node) {
		if (func(node->item, data) > 0) {
			return node;
		}
		node = node->next;
	}
	return NULL;
}

MemberNode member_last(MemberNode node) {
	while (node) {
		if (node->next == NULL) {
			return node;
		}
		node = node->next;
	}
	return NULL;
}

int find_by_score(int score, MemberNode members, int query) {
	return score == query;
}

int find_item(int item, int query) {
	return item == query;
}

#ifdef DEBUG
int pool_print(int score, int count, MemberNode members) {
	printf("Pool %d (count %d)\n", score, count);
	member_foreach(members, member_print);
	return 0;
}

int member_print(int item) {
	printf(" -> item %d\n", item);
	return 0;
}
#endif

PoolNode preparePromotion(PoolNode head, int item, int score) {
	MemberNode memberMatch;
	if (head->score == score) {
		if (head->members->item == item) {
			memberMatch = head->members;
			head->members = memberMatch->next;
			head->count -= 1;
		} else {
			if ((memberMatch = member_find(head->members, find_item, item))) {
				head->count -= 1;
				assert(member_remove(head->members, memberMatch) == 0);
			}
		}
		if (head->count == 0) {
			app_stats.pools -= 1;
			return head->next;
		}
	} else {
		PoolNode listMatch;
		if ((listMatch = pool_find(head, find_by_score, score))) {
			if (listMatch->count == 1) {
				assert(pool_remove(head, listMatch) == 0);
			} else {
				if (listMatch->members->item == item) {
					memberMatch = listMatch->members;
					listMatch->members = memberMatch->next;
					listMatch->count -= 1;
				} else {
					if ((memberMatch = member_find(listMatch->members, find_item, item))) {
						listMatch->count -= 1;
						assert(member_remove(listMatch->members, memberMatch) == 0);
					} else {
						assert(1);
					}
				}
			}
		} else {
			assert(1);
		}
	}
	return head;
}

PoolNode promoteItem(PoolNode list, int score, int item, int old_score) {
	if (old_score != -1) {
		list = preparePromotion(list, item, old_score);
	}
	if (list == NULL) {
		PoolNode newPool = pool_create(score);
		MemberNode newMember;
		newMember = member_create(item);
		newPool->members = newMember;
		newPool->count++;
		return newPool;
	}
	PoolNode listMatch;
	if ((listMatch = pool_find(list, find_by_score, score))) {
		MemberNode memberMatch;
		if (! (memberMatch = member_find(listMatch->members, find_item, item))) {
			listMatch->members = member_push(listMatch->members, item);
			listMatch->count++;
		}
		return list;
	} else {
		// Score pool doesn't exist
		PoolNode newPool;
		newPool = pool_create(score);
		app_stats.pools += 1;
		MemberNode newMember;
		newMember = member_create(item);
		newPool->members = newMember;
		newPool->count++;
		if (list->score < score) {
			// The new pool is larger than the head, set as new head
			newPool->next = list;
			return newPool;
		} else if (list->next == NULL && list->score > score) {
			// There is only one member in the head and the head is larger
			// than score, set head->next to newPool and return head.
			list->next = newPool;
			return list;
		} else {
			// There are more than two items in the list, find where to
			// create the new pool
			PoolNode head = list;
			while (head) {
				if (score < head->score) {
					if (head->next == NULL) {
						head->next = newPool;
						newPool->next = NULL;
						break;
					}
					if (head->next != NULL && score < head->score && score > head->next->score) {
						PoolNode tmp;
						tmp = head->next;
						newPool->next = tmp;
						head->next = newPool;
						break;
					}
				}
				head = head->next;
			}
			return list;
		}
	}
	return NULL;
}

void PeekNext(PoolNode head, int *next_item) {
	if (head == NULL) {
		*next_item = -1;
		return;
	}
	if (head->count == 1) {
		*next_item = head->members->item;
		return;
	} else {
		MemberNode last = member_last(head->members);
		*next_item = last->item;
		return;
	}
}

PoolNode NextItem(PoolNode head, int *next_item) {
	if (head == NULL) {
		*next_item = -1;
		return NULL;
	}
	if (head->count == 1) {
		*next_item = head->members->item;
		app_stats.pools -= 1;
		return head->next;
	} else {
		MemberNode last = member_last(head->members);
		*next_item = last->item;
		assert(member_remove(head->members, last) == 0);
		head->count -= 1;
		return head;
	}
}
