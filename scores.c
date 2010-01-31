#include "scores.h"
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

ScoreBucket PurgeThenAddScoreToPool(ScoreBucket bucket, int score, int item_id, int old_score) {
    ScoreBucket lookup = doesPoolExist(bucket, old_score);
    lookup->members = DeleteMember(lookup->members, item_id);
    lookup->count -= 1;
    return AddScoreToPool(bucket, score, item_id);
}

ScoreBucket AddScoreToPool(ScoreBucket bucket, int score, int item_id) {
    if (bucket == NULL) {
        return initScorePool(score, item_id);
    }
    ScoreBucket lookup = doesPoolExist(bucket, score);
    if (lookup == NULL) {
        ScoreBucket head = initScorePool(score, item_id);
        head->next = bucket;
        return head;
    }
    lookup = AddScoreMember(lookup, item_id);
    return bucket;
}

ScoreBucket initScorePool(int score, int item_id) {
    ScoreBucket head = malloc(sizeof(ScoreBucket));
    if (head == NULL ) {
        exit(1);
    } else {
        MemberBucket member = malloc( sizeof(MemberBucket ) );
        member->item = item_id;
        member->next = NULL;
        head->members = member;
        head->score = score;
        head->count = 1;
        head->next = NULL;
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
    if (head->item == item) { return head->next; }
    head->next = DeleteMember(head->next, item);
    return head;
}
