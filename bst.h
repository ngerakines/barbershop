#ifndef _Tree_H
#define _Tree_H

struct TreeNode;
typedef struct TreeNode *Position;
typedef struct TreeNode *SearchTree;

SearchTree MakeEmpty(SearchTree tree);
Position Find(int key, SearchTree tree);
Position FindMin(SearchTree tree);
Position FindMax(SearchTree tree);
SearchTree Insert(int key, int score, SearchTree tree);
SearchTree Delete(int key, SearchTree tree);
int RetrieveItem(Position P);
int RetrieveScore(Position P);
void DumpItems(SearchTree tree);

#endif
