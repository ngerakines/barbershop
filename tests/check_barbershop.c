#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <assert.h>
#include "../src/scores.h"

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

START_TEST (test_scores_empty) {
	ScoreBucket bucket_a = PrepScoreBucket(NULL);
	fail_unless(bucket_a == NULL, "empty buckets aren't anything.");
	fail_unless(GetNextItem(bucket_a) == -1, "Empty buckets return no items.");
} END_TEST

// Assert that counts are maintained while adding.
START_TEST (test_scores_add) {
	ScoreBucket bucket_b = PrepScoreBucket(NULL);
	bucket_b = AddScoreToPool(bucket_b, 1, 5000);
	fail_unless(bucket_b->score == 1, "Adding items to buckets is reflected in the bucket score.");
	fail_unless(bucket_b->count == 1, "Adding items to buckets is reflected in the bucket count.");
	fail_unless(IsScoreMember(bucket_b->members, 5000) == 1, "Item 5000 is in bucket_b.");
	fail_unless(IsScoreMember(bucket_b->members, 5001) == 0, "Item 5001 is not in bucket_b.");
	fail_unless(GetNextItem(bucket_b) == 5000, "Next returns item 5000.");
	fail_unless(IsScoreMember(bucket_b->members, 5000) == 0, "Item 5000 was pulled from bucket_b.");
	fail_unless(bucket_b->members == NULL, "has no members");
	fail_unless(bucket_b->count == 0, "has count of 0");
	fail_unless(bucket_b->next == NULL, "next goes nowhere.");
} END_TEST

// Assert insert order is maintained.
START_TEST (test_scores_add_several) {
	ScoreBucket bucket_c = PrepScoreBucket(NULL);
	bucket_c = AddScoreToPool(bucket_c, 1, 5000);
	bucket_c = AddScoreToPool(bucket_c, 1, 5001);
	bucket_c = AddScoreToPool(bucket_c, 1, 5002);
	fail_if(bucket_c->members == NULL, "has members");
	fail_unless(bucket_c->count == 3, "has count of 3");
	fail_unless(GetNextItem(bucket_c) == 5000, "Next returns item 5000.");
	fail_unless(GetNextItem(bucket_c) == 5001, "Next returns item 5001.");
	fail_unless(GetNextItem(bucket_c) == 5002, "Next returns item 5002.");
	fail_unless(bucket_c->members == NULL, "has no members left");
	fail_unless(bucket_c->count == 0, "has count of 0");
} END_TEST

// Assert promoting ensures accurate counts and membership
START_TEST (test_scores_promote) {
	ScoreBucket bucket_d = PrepScoreBucket(NULL);
	bucket_d = AddScoreToPool(bucket_d, 1, 5000);
	fail_unless(bucket_d->count == 1, "bucket_d[0] has one item");
	fail_unless(bucket_d->score == 1, "bucket_d[0] score is 1");
	bucket_d = PurgeThenAddScoreToPool(bucket_d, 2, 5000, 1);

	fail_unless(IsScoreMember(bucket_d->members, 5000) == 1, "bucket_d[0] has item 5000");

	printf("Dumping score buckets:\n");
	DumpScores(bucket_d);

	fail_unless(bucket_d->count == 1, "bucket_d[1] has one item");
	fail_unless(bucket_d->next->count == 0, "bucket_d[0] has no items");
	fail_unless(bucket_d->score == 2, "bucket_d[0] score is 2");
	bucket_d = AddScoreToPool(bucket_d, 1, 5001);
	fail_unless(bucket_d->next->count == 1, "bucket_d[1] has one item");

	fail_unless(GetNextItem(bucket_d) == 5000, "Next returns item 5000.");
	fail_unless(GetNextItem(bucket_d) == 5001, "Next returns item 5001.");
} END_TEST

Suite * barbershop_suite(void) {
	Suite *s = suite_create("Barbershop");

	TCase *tc_core = tcase_create("Core");
	// tcase_add_test(tc_core, test_scores_empty);
	// tcase_add_test(tc_core, test_scores_add);
	// tcase_add_test(tc_core, test_scores_add_several);
	tcase_add_test(tc_core, test_scores_promote);
	suite_add_tcase(s, tc_core);

	return s;
}

int main (void) {
	int number_failed;
	Suite *s = barbershop_suite();
	SRunner *sr = srunner_create(s);
	srunner_run_all(sr, CK_VERBOSE);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
