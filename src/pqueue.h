/*
 * File:   pqueue.h
 * Author: dwayn
 *
 * Created on June 25, 2010, 11:45 PM
 */

#ifndef _PQUEUE_H
#define	_PQUEUE_H

struct item_node;
struct item_tree_node;
struct score_tree_node;
typedef struct item_node *ItemNode;
typedef struct item_tree_node *ItemTreeNode;
typedef struct score_tree_node *ScoreTreeNode;


//Items linked list to keep each pool attached together
struct item_node {
	int itemId;
	struct item_node *prev;
	struct item_node *next;
};

//bst to lookup score pools by score
struct score_tree_node {
    struct score_tree_node *left;
    struct score_tree_node *right;
    struct item_node *head;
    struct item_node *tail;
    int score;
};

//bst to lookup items by item id
struct item_tree_node {
    struct item_tree_node *left;
    struct item_tree_node *right;
    struct item_node *item;
    int score;
};


ScoreTreeNode score_root;
ItemTreeNode item_root;

/**
 * These are the functions to access the priority queue
 */
// returns the itemId of the item on the top of the queue and leaves item in queue or -1 if queue is empty
int peekNext();
// returns the itemId of the item on the top of the queue and removes item from queue or -1 if queue is empty
int getNext();
// returns the score of a specific itemId or -1 if the item is not in the queue
int getScore(int itemId);
// returns 1 on success, 0 on failure
int update(int itemId, int score);
// iterates through scores and outputs them in the format "itemId score\r\n" to the given file.
// pass NULL for tree to start at the root of the tree
void outputScores(FILE *fd);
void outputScoresIterator(FILE *fd, ScoreTreeNode tree);
void initializePriorityQueue();
void emptyPriorityQueue();

void dumpItems();
void dumpItemsIterator(ItemTreeNode tree);
void dumpScores();
void dumpScoresIterator(ScoreTreeNode tree);

/**
 * All functions below are only used internally by the above functions
 */
ScoreTreeNode createScoreTreeNode(int score);
ItemNode createItemNode(int id);
ItemTreeNode createItemTreeNode();


void addItemNode(ScoreTreeNode score, ItemNode i);
ScoreTreeNode addScoreTreeNode(ScoreTreeNode tree, ScoreTreeNode node);
ItemTreeNode addItemTreeNode(ItemTreeNode tree, ItemTreeNode node);

// requires the node to not be attached to a list (call removeItemNode() to detach node)
void deleteItemNode(ItemNode i);
int removeItemNode(ScoreTreeNode list, ItemNode i);
ItemTreeNode deleteItemTreeNode(ItemTreeNode tree, ItemTreeNode node);
ScoreTreeNode deleteScoreTreeNode(ScoreTreeNode tree, ScoreTreeNode node);


ScoreTreeNode findMinScore(ScoreTreeNode node);
ScoreTreeNode findMaxScore(ScoreTreeNode node);
ItemTreeNode findMinItem(ItemTreeNode node);
ItemTreeNode findMaxItem(ItemTreeNode node);


ScoreTreeNode findScore(int score, ScoreTreeNode tree);
ItemTreeNode findItem(int itemId, ItemTreeNode tree);


#endif	/* _PQUEUE_H */

