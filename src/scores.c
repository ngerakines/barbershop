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

int GetNextItem(ScoreBucket head) {
	if (head == NULL) { return -1; }
	if (head->count > 0) {
		// This shouldn't be a two step process, O(n^2) -> 0(n)
		MemberBucket last = ReturnLastMember(head->members);
		head->members = DeleteMember(head->members, last->item);
		// If the pool has no members, decr pools_gc stats to reflect
		// non-garbage collected pool counts.
		if (head->members == NULL) {
			app_stats.pools_gc -= 1;
		}
		head->count -= 1;
		int item_id = last->item;
		free(last);
		return item_id;
	}
	return GetNextItem(head->next);
}

ScoreBucket PurgeThenAddScoreToPool(ScoreBucket bucket, int score, int item_id, int old_score) {
	ScoreBucket lookup = doesPoolExist(bucket, old_score);
	lookup->members = DeleteMember(lookup->members, item_id);
	assert(IsScoreMember(lookup->members, item_id) == 0);
	lookup->count -= 1;
	if (lookup->members == NULL) {
		app_stats.pools_gc -= 1;
	}
	return AddScoreToPool(bucket, score, item_id);
}

// TODO: This should be a self-sorting linked-list. All new pools
// should be injected to retain uniqueness and order.
ScoreBucket AddScoreToPool(ScoreBucket bucket, int score, int item_id) {
	printf("Adding %d to pool %d\n", item_id, score);
	ScoreBucket lookup = doesPoolExist(bucket, score);
	if (lookup == NULL) {
		ScoreBucket head = initScorePool(score, item_id);
		assert(IsScoreMember(head->members, item_id) == 1);
		head->next = bucket;
		return head;
	}
	lookup = AddScoreMember(lookup, item_id);
	assert(IsScoreMember(lookup->members, item_id) == 1);
	return lookup;
}

// TODO: Look into merging this into `AddScoreMember/2`
ScoreBucket initScorePool(int score, int item_id) {
	ScoreBucket head = malloc(sizeof(ScoreBucket));
	if (head == NULL ) {
		exit(1);
	} else {
		MemberBucket member = malloc(sizeof(MemberBucket));
		member->item = item_id;
		member->next = NULL;
		head->members = member;
		head->score = score;
		head->count = 1;
		// Do it here we we can effectively see if the pool should be
		// created for the first time but may not have for one reason
		// or another.
		app_stats.pools += 1;
		app_stats.pools_gc += 1;
		return head;
	}
	return NULL;
}

ScoreBucket AddScoreMember(ScoreBucket bucket, int item) {
    if (IsScoreMember(bucket->members, item)) {
        return bucket;
    }
    MemberBucket member = malloc(sizeof(MemberBucket));
    if (member == NULL ) {
        exit(1);
    } else {
        member->next = bucket->members;
        member->item = item;
        bucket->members = member;
        bucket->count += 1;
        return bucket;
    }
    return NULL;
}

ScoreBucket PrepScoreBucket(ScoreBucket bucket) {
    if (bucket != NULL) {
        free(bucket);
    }
    return NULL;
}

int IsScoreMember(MemberBucket head, int item) {
    if (head == NULL) { return 0; }
    if (head->item == item) {return 1; }
    return IsScoreMember(head->next, item);
}

ScoreBucket doesPoolExist(ScoreBucket bucket, int score) {
    if (bucket == NULL) { return NULL; }
    if (bucket->score == score) { return bucket; }
    return doesPoolExist(bucket->next, score);
}

void DumpScores(ScoreBucket head) {
    if (head == NULL) { return; }
    printf("Score %d (%d)", head->score, head->count);
    DumpMembers(head->members);
    printf("\n");
    DumpScores(head->next);
}

void DumpMembers(MemberBucket head) {
    if (head == NULL) { return; }
    printf(" %d", head->item);
    DumpMembers(head->next);
}

MemberBucket DeleteMember(MemberBucket head, int item) {
	if (head == NULL) { return NULL; }
	if (head->item == item) {
		return head->next;
	} else {
		head->next = DeleteMember(head->next, item);
	}
	return head;
}

MemberBucket ReturnLastMember(MemberBucket head) {
	assert(head != NULL);
	if (head->next == NULL) {
		return head;
	}
	return ReturnLastMember(head->next);
}
