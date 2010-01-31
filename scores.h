#ifndef __SCORES_H__
#define __SCORES_H__

struct member_el;
struct bucket_el;
typedef struct member_el *MemberBucket;
typedef struct bucket_el *ScoreBucket;

ScoreBucket PurgeThenAddScoreToPool(ScoreBucket bucket, int score, int item_id, int old_score);
ScoreBucket AddScoreToPool(ScoreBucket bucket, int score, int item_id);
ScoreBucket initScorePool(int score, int item_id);
ScoreBucket AddScoreMember(ScoreBucket bucket, int item);
ScoreBucket PrepScoreBucket(ScoreBucket bucket);
int IsScoreMember(MemberBucket head, int item);
ScoreBucket doesPoolExist(ScoreBucket bucket, int score);
void DumpScores(ScoreBucket head);
void DumpMembers(MemberBucket head);
MemberBucket DeleteMember(MemberBucket head, int item);

#endif
