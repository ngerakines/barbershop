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
int GetNextItem(ScoreBucket head);

#endif
