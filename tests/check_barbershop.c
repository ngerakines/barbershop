#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <assert.h>
#include "../src/scores.h"

START_TEST (test_pools_empty) {
	PoolNode *a = NULL;
	fail_unless(a == NULL, "empty pools are NULL.");
	int next;
	a = NextItem(a, &next);
	fail_unless(next == -1, "Empty buckets return no items.");
} END_TEST

// Assert that counts are maintained while adding.
START_TEST (test_pools_add) {
	PoolNode *head_b = NULL;
	int next = -1;
	head_b = promoteItem(head_b, 1, 5000, -1);
	fail_if(head_b->score != 1);
	fail_if(head_b->count != 1);
	head_b = NextItem(head_b, &next);
	printf("next %d\n", next);
	fail_unless(next == 5000);
	pool_foreach(head_b, pool_print);
	fail_if(head_b->score != 1);
	fail_if(head_b->count != 0);
	head_b = NextItem(head_b, &next);
	printf("next %d\n", next);
	fail_unless(next == -1);
	fail_if(head_b->score != 1);
	fail_if(head_b->count != 0);
} END_TEST
// 
// // Assert insert order is maintained.
// START_TEST (test_pools_add_several) {
// 	PoolNode *head_c = NULL;
// 	head_c = promoteItem(head_c, 1, 5000, -1);
// 	fail_if(head_c->score != 1);
// 	fail_if(head_c->count != 1);
// 	head_c = promoteItem(head_c, 1, 5001, -1);
// 	fail_if(head_c->count != 2);
// 	head_c = promoteItem(head_c, 1, 5002, -1);
// 	fail_if(head_c->score != 1);
// 	fail_if(head_c->count != 3);
// 	fail_unless(NextItem(head_c) == 5000);
// 	fail_if(head_c->count != 2);
// 	fail_unless(NextItem(head_c) == 5001);
// 	fail_if(head_c->count != 1);
// 	fail_unless(NextItem(head_c) == 5002);
// 	fail_if(head_c->count != 0);
// } END_TEST
// 
// // Assert promoting ensures accurate counts and membership
// START_TEST (test_pools_promote) {
// 	PoolNode *head_d = NULL;
// 	head_d = promoteItem(head_d, 1, 5000, -1);
// 	head_d = promoteItem(head_d, 1, 5001, -1);
// 	head_d = promoteItem(head_d, 2, 5000, 1);
// 	head_d = promoteItem(head_d, 3, 5000, 2);
// 	fail_if(head_d->score != 3);
// 	fail_if(head_d->count != 1);
// 	fail_if(head_d->next->score != 2);
// 	fail_if(head_d->next->count != 0);
// 	fail_if(head_d->next->next->score != 1);
// 	fail_if(head_d->next->next->count != 1);
// } END_TEST

Suite * barbershop_suite(void) {
	Suite *s = suite_create("Barbershop");
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, test_pools_empty);
	tcase_add_test(tc_core, test_pools_add);
	// tcase_add_test(tc_core, test_pools_add_several);
	// tcase_add_test(tc_core, test_pools_promote);
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
