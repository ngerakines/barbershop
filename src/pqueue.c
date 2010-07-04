/*
Copyright (c) 2010 Dwayn Matthies <dwayn(dot)matthies(at)gmail(dot)com>

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

#include <stdlib.h>
#include <stdio.h>
#include "pqueue.h"
#include "stats.h"

void initializePriorityQueue()
{
	score_root = NULL;
	item_root = NULL;
}

int peekNext()
{
	ScoreTreeNode snode = findMaxScore(score_root);
	if(snode == NULL)
		return -1;
	return snode->head->itemId;
}

int getNext()
{
	ScoreTreeNode snode = findMaxScore(score_root);
	if(snode == NULL)
		return -1;
	int rval;
	rval = snode->head->itemId;
	ItemTreeNode itnode = findItem(snode->head->itemId, item_root);
	int populated = removeItemNode(snode, itnode->item);
	if(!populated)
	{
		score_root = deleteScoreTreeNode(score_root, snode);
	}
	itnode->score = 0;
	ItemNode tmp = itnode->item;
	item_root = deleteItemTreeNode(item_root, itnode);
	deleteItemNode(tmp);

	return rval;
}

int getScore(int itemId)
{
	ItemTreeNode itnode = findItem(itemId, item_root);
	if(itnode == NULL)
		return -1;
	return itnode->score;
}

// returns 1 on adding item, 0 on successful update of item, -1 on error
int update(int itemId, int score)
{
	int rval = 0;
	int newscore = score;
    ItemTreeNode itnode = findItem(itemId, item_root);
    if(itnode == NULL)
    {
		rval = 1;
        itnode = createItemTreeNode();
		if(itnode == NULL)
			return -1;
        itnode->item = createItemNode(itemId);
		if(itnode->item == NULL)
			return -1;
		itnode->score = 0;
        item_root = addItemTreeNode(item_root, itnode);
    }

	ScoreTreeNode snode;
    if(itnode->score > 0)
    {
	    snode = findScore(itnode->score, score_root);
		newscore = score + itnode->score;
		//printf("moving itemid %d to score %d from score %d\n", itnode->item->itemId, newscore, snode->score);
		int populated = removeItemNode(snode, itnode->item);
		if(!populated)
		{
			score_root = deleteScoreTreeNode(score_root, snode);
		}
		itnode->score = 0;
    }
	snode = findScore(newscore, score_root);
	if(snode == NULL)
	{
		snode = createScoreTreeNode(newscore);
		if(snode == NULL)
			return -1;
		score_root = addScoreTreeNode(score_root, snode);
	}
	addItemNode(snode, itnode->item);
	itnode->score = snode->score;
	app_stats.updates += 1;
	return rval;
}


void outputScores(FILE *fd)
{
	outputScoresIterator(fd, score_root);
}
void outputScoresIterator(FILE *fd, ScoreTreeNode tree)
{
	if(tree == NULL)
		return;
	ItemNode i = tree->head;
	while(i)
	{
		fprintf(fd, "%d %d\n", i->itemId, tree->score);
		i = i->next;
	}
	outputScoresIterator(fd, tree->left);
	outputScoresIterator(fd, tree->right);
}


void dumpItems()
{
	dumpItemsIterator(item_root);
}

void dumpItemsIterator(ItemTreeNode tree)
{
	if(tree == NULL)
		return;
	dumpItemsIterator(tree->left);
	printf("<- %d ->", tree->item->itemId);
	dumpItemsIterator(tree->right);
}

void dumpScores()
{
	dumpScoresIterator(score_root);
}

void dumpScoresIterator(ScoreTreeNode tree)
{
	if(tree == NULL)
		return;
	dumpScoresIterator(tree->left);
	printf("<- %d ->", tree->score);
	dumpScoresIterator(tree->right);
}


/**
 * All functions below are only used internally by the above functions
 */
ScoreTreeNode createScoreTreeNode(int score)
{
	ScoreTreeNode node;
	if(!(node = malloc(sizeof(struct score_tree_node))))
		return NULL;
	node->score = score;
	node->head = NULL;
	node->tail = NULL;
	node->left = NULL;
	node->right = NULL;
	return node;
}
ItemNode createItemNode(int id)
{
	ItemNode node;
	if(!(node = malloc(sizeof(struct item_node))))
		return NULL;
	node->itemId = id;
	node->next = NULL;
	node->prev = NULL;
	return node;
}
ItemTreeNode createItemTreeNode()
{
	ItemTreeNode node;
	if (! (node = malloc(sizeof(struct item_tree_node))))
		return NULL;
	node->left = NULL;
	node->right = NULL;
	node->item = NULL;
	node->score = 0;
	return node;

}


void addItemNode(ScoreTreeNode score, ItemNode i)
{
	if(score->head == NULL)
	{
		score->head = i;
		score->tail = i;
		return;
	}

	score->tail->next = i;
	i->prev = score->tail;
	score->tail = i;
}
ScoreTreeNode addScoreTreeNode(ScoreTreeNode tree, ScoreTreeNode node)
{
	if(tree == NULL)
	{
		tree = node;
		app_stats.pools += 1;
	}
	else
	{
		if(node->score < tree->score)
		{
			tree->left = addScoreTreeNode(tree->left, node);
		}
		else if(node->score > tree->score)
		{
			tree->right = addScoreTreeNode(tree->right, node);
		}
	}
	return tree;
}
ItemTreeNode addItemTreeNode(ItemTreeNode tree, ItemTreeNode node)
{
	if(tree == NULL)
	{
		tree = node;
		app_stats.items += 1;
	}
	else
	{
		if(node->item->itemId < tree->item->itemId)
		{
			tree->left = addItemTreeNode(tree->left, node);
		}
		else if(node->item->itemId > tree->item->itemId)
		{
			tree->right = addItemTreeNode(tree->right, node);
		}
	}
	return tree;
}


void deleteItemNode(ItemNode i)
{
	if(i->next == NULL && i->prev == NULL)
	{
		free(i);
	}
	else
	{
		printf("Error: tried to free ItemNode that was still attached to others\n");
		exit(1);
	}
}

//returns 1 if there is still items in the list, 0 if the list is empty
int removeItemNode(ScoreTreeNode list, ItemNode i)
{
	if(list->head == i && list->tail == i)
	{
		list->head = NULL;
		list->tail = NULL;
		return 0;
	}

	if(list->head == i)
	{
		list->head = list->head->next;
		list->head->prev = NULL;
		i->next = NULL;
		return 1;
	}

	if(list->tail == i)
	{
		list->tail = list->tail->prev;
		list->tail->next = NULL;
		i->prev = NULL;
		return 1;
	}

	i->next->prev = i->prev;
	i->prev->next = i->next;
	i->prev = NULL;
	i->next = NULL;
	return 1;
}
ItemTreeNode deleteItemTreeNode(ItemTreeNode tree, ItemTreeNode node)
{
	if(tree == NULL)
	{
		printf("Error: can't find ScoreTreeNode to delete\n");
		exit(1);
	}

	ItemTreeNode TmpCell;

	if(node->item->itemId < tree->item->itemId)
	{
		tree->left = deleteItemTreeNode(tree->left, node);
	}
	else if(node->item->itemId > tree->item->itemId)
	{
		tree->right = deleteItemTreeNode(tree->right, node);
	}
	else if(tree->left && tree->right)
	{
		ItemNode TmpItem;
		TmpCell = findMinItem(tree->right);
		TmpItem = tree->item;
		tree->item = TmpCell->item;
		tree->score = TmpCell->score;
		TmpCell->item = TmpItem;
		TmpCell->score = 0;
		tree->right = deleteItemTreeNode(tree->right, TmpCell);
	}
	else
	{
		TmpCell = tree;
		if(tree->left == NULL)
		{
			tree = tree->right;
		}
		else if(tree->right == NULL)
		{
			tree = tree->left;
		}
		free(TmpCell);
		app_stats.items -= 1;
	}

	return tree;
}
ScoreTreeNode deleteScoreTreeNode(ScoreTreeNode tree, ScoreTreeNode node)
{
	if(!(node->head == NULL && node->tail == NULL))
	{
		printf("Error: tried to free ScoreTreeNode that still has items in its list\n");
		exit(1);
	}
	if(tree == NULL)
	{
		printf("Error: can't find ScoreTreeNode to delete\n");
		exit(1);
	}
	ScoreTreeNode TmpCell;
	if(node->score < tree->score)
	{
		tree->left = deleteScoreTreeNode(tree->left, node);
	}
	else if(node->score > tree->score)
	{
		tree->right = deleteScoreTreeNode(tree->right, node);
	}
	else if(tree->left && tree->right)
	{
		int tmpscore;
		TmpCell = findMinScore(tree->right);
		tree->head = TmpCell->head;
		tree->tail = TmpCell->tail;
		tmpscore = tree->score;
		tree->score = TmpCell->score;
		TmpCell->score = tmpscore;
		TmpCell->head = NULL;
		TmpCell->tail = NULL;
		tree->right = deleteScoreTreeNode(tree->right, TmpCell);
	}
	else
	{
		TmpCell = tree;
		if(tree->left == NULL)
		{
			tree = tree->right;
		}
		else if(tree->right == NULL)
		{
			tree = tree->left;
		}
		free(TmpCell);
		app_stats.pools -= 1;
	}

	return tree;
}

ScoreTreeNode findMinScore(ScoreTreeNode node)
{
	if(node == NULL)
		return NULL;
	ScoreTreeNode TmpCell = node;
	while(TmpCell->left != NULL)
	{
		TmpCell = TmpCell->left;
	}
	return TmpCell;
}
ScoreTreeNode findMaxScore(ScoreTreeNode node)
{
	if(node == NULL)
		return NULL;
	ScoreTreeNode TmpCell = node;
	while(TmpCell->right != NULL)
	{
		TmpCell = TmpCell->right;
	}
	return TmpCell;
}
ItemTreeNode findMinItem(ItemTreeNode node)
{
	if(node == NULL)
		return NULL;
	ItemTreeNode TmpCell = node;
	while(TmpCell->left != NULL)
	{
		TmpCell = TmpCell->left;
	}
	return TmpCell;
}
ItemTreeNode findMaxItem(ItemTreeNode node)
{
	if(node == NULL)
		return NULL;
	ItemTreeNode TmpCell = node;
	while(TmpCell->right != NULL)
	{
		TmpCell = TmpCell->right;
	}
	return TmpCell;
}

ScoreTreeNode findScore(int score, ScoreTreeNode tree)
{
	if(tree == NULL)
		return NULL;
	if(score < tree->score)
	{
		return findScore(score, tree->left);
	}
	else if(score > tree->score)
	{
		return findScore(score, tree->right);
	}
	return tree;
}
ItemTreeNode findItem(int itemId, ItemTreeNode tree)
{
	if(tree == NULL)
		return NULL;
	if(itemId < tree->item->itemId)
	{
		return findItem(itemId, tree->left);
	}
	else if(itemId > tree->item->itemId)
	{
		return findItem(itemId, tree->right);
	}
	return tree;
}

