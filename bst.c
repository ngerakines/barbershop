#include "bst.h"
#include <stdlib.h>
#include <stdio.h>

struct TreeNode {
    int item;
    int score;
    SearchTree left;
    SearchTree right;
};

SearchTree MakeEmpty(SearchTree tree) {
    if (tree != NULL) {
        MakeEmpty(tree->left);
        MakeEmpty(tree->right);
        free(tree);
    }
    return NULL;
}

Position Find(int key, SearchTree tree) {
    if (tree == NULL) {
        return NULL;
    }
    if (key < tree->item) {
        return Find(key, tree->left);
    } else if (key > tree->item) {
        return Find(key, tree->right);
    }
    return tree;
}

Position FindMin(SearchTree tree) {
    if (tree == NULL) {
        return NULL;
    } else if (tree->left == NULL) {
        return tree;
    }
    return FindMin(tree->left);
}

Position FindMax(SearchTree tree) {
    if (tree != NULL) {
        while (tree->right != NULL) {
            tree = tree->right;
        }
    }
    return tree;
}

void DumpItems(SearchTree tree) {
    if (tree == NULL) { return; }
    DumpItems(tree->left);
    printf("%d (%d) - %d - %d\n", tree->item, tree->score,
        tree->left ? tree->left->item : -1,
        tree->right ? tree->right->item : -1);
    DumpItems(tree->right);
}

SearchTree Insert(int key, int score, SearchTree tree ) {
    if ( tree == NULL) {
        tree = malloc( sizeof( struct TreeNode ) );
        if (tree == NULL ) {
            // FatalError( "Out of space!!!" );
        } else {
            tree->item = key;
            tree->score = score;
            tree->left = tree->right = NULL;
        }
    } else {
        if ( key < tree->item ) {
            tree->left = Insert(key, score, tree->left );
        } else if( key > tree->item ) {
            tree->right = Insert(key, score, tree->right );
        }
    }
    return tree;
}

SearchTree Delete(int key, SearchTree tree) {
    Position TmpCell;
    if (tree == NULL) {
        // Error( "Element not found" );
    } else {
        if (key < tree->item) { /* Go left */
            tree->left = Delete(key, tree->left );
        } else if ( key > tree->item) { /* Go right */
            tree->right = Delete(key, tree->right );
        } else if (tree->left && tree->right) { /* Two children */
            /* Replace with smallest in right subtree */
            TmpCell = FindMin(tree->right);
            tree->item = TmpCell->item;
            tree->right = Delete(tree->item, tree->right );
        } else { /* One or zero children */
            TmpCell = tree;
            if (tree->left == NULL) { /* Also handles 0 children */
                tree = tree->right;
            } else if (tree->right == NULL) {
                tree = tree->left;
            }
            free( TmpCell );
        }
    }
    return tree;
}

int RetrieveItem(Position P) {
    return P->item;
}

int RetrieveScore(Position P) {
    return P->item;
}
