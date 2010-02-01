#include <stdlib.h>
#include <check.h>
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

START_TEST (test_scores_add_several) {
	ScoreBucket bucket_b = PrepScoreBucket(NULL);
	bucket_b = AddScoreToPool(bucket_b, 1, 5000);
	bucket_b = AddScoreToPool(bucket_b, 1, 5001);
	bucket_b = AddScoreToPool(bucket_b, 1, 5002);
	fail_if(bucket_b->members == NULL, "has members");
	fail_unless(bucket_b->count == 3, "has count of 0");
	// TODO: These tests fail because there is a bug in the return order. When
	// members are added they are added to the head but when `next` is called
	// they are pulled from the head instead of from the last member of the chain.
	fail_unless(GetNextItem(bucket_b) == 5000, "Next returns item 5000.");
	fail_unless(GetNextItem(bucket_b) == 5001, "Next returns item 5001.");
	fail_unless(GetNextItem(bucket_b) == 5002, "Next returns item 5002.");
} END_TEST

START_TEST (test_scores_promote) {
	ScoreBucket bucket_c = PrepScoreBucket(NULL);
	bucket_c = AddScoreToPool(bucket_c, 1, 5000);
	// TODO: Assert that item is member 1 of scores head
	bucket_c = PurgeThenAddScoreToPool(bucket_c, 2, 5000, 1);
	// TODO: Assert that item is member 1 of scores head
	// TODO: Assert that head count is 1 and head->next count is 0
	fail_unless(bucket_c->next->score == 1, "Adding items to new buckets bumps the chain");
	fail_unless(bucket_c->score == 2, "Adding items to new buckets bumps the chain");
} END_TEST

Suite * barbershop_suite(void) {
	Suite *s = suite_create("Barbershop");

	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, test_scores_empty);
	tcase_add_test(tc_core, test_scores_add);
	tcase_add_test(tc_core, test_scores_add_several);
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
