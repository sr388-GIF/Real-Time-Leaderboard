#include "avl.h"
#include "player.h"
#include <stdlib.h>
#include <string.h>

/* Comparison: returns -1 if a < b, 0 if equal, 1 if a > b.
 * Higher score first; on equal score, smaller id first. */
static int cmp_players(const Player *a, const Player *b) {
    if (a->score != b->score)
        return (a->score > b->score) ? 1 : -1;
    if (a->id != b->id)
        return (a->id < b->id) ? 1 : -1;  /* smaller id = higher rank */
    return 0;
}

static int height(const AVLNode *n) {
    return n ? n->height : 0;
}

static int max_int(int a, int b) {
    return (a > b) ? a : b;
}

static AVLNode *new_node(const Player *p) {
    AVLNode *n = (AVLNode *)malloc(sizeof(AVLNode));
    if (!n) return NULL;
    n->player = player_clone(p);
    n->left = n->right = NULL;
    n->height = 1;
    return n;
}

static void free_node(AVLNode *n) {
    if (!n) return;
    player_free(n->player);
    free(n);
}

static AVLNode *rotate_right(AVLNode *y) {
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = 1 + max_int(height(y->left), height(y->right));
    x->height = 1 + max_int(height(x->left), height(x->right));
    return x;
}

static AVLNode *rotate_left(AVLNode *x) {
    AVLNode *y = x->right;
    AVLNode *T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = 1 + max_int(height(x->left), height(x->right));
    y->height = 1 + max_int(height(y->left), height(y->right));
    return y;
}

static int get_balance(const AVLNode *n) {
    return n ? height(n->left) - height(n->right) : 0;
}

static AVLNode *insert_rec(AVLNode *root, const Player *p, int *status) {
    if (!root) {
        *status = 0;
        return new_node(p);
    }

    int cmp = cmp_players(p, root->player);
    if (cmp == 0) {
        *status = -1;  /* duplicate */
        return root;
    }

    if (cmp > 0)
        root->left = insert_rec(root->left, p, status);
    else
        root->right = insert_rec(root->right, p, status);

    if (*status != 0) return root;

    root->height = 1 + max_int(height(root->left), height(root->right));
    int balance = get_balance(root);

    /* LL */
    if (balance > 1 && cmp_players(p, root->left->player) > 0)
        return rotate_right(root);
    /* RR */
    if (balance < -1 && cmp_players(p, root->right->player) < 0)
        return rotate_left(root);
    /* LR */
    if (balance > 1 && cmp_players(p, root->left->player) < 0) {
        root->left = rotate_left(root->left);
        return rotate_right(root);
    }
    /* RL */
    if (balance < -1 && cmp_players(p, root->right->player) > 0) {
        root->right = rotate_right(root->right);
        return rotate_left(root);
    }
    return root;
}

int avl_insert(AVLTree *tree, const Player *p) {
    int status = 0;
    tree->root = insert_rec(tree->root, p, &status);
    if (status == 0) tree->size++;
    return status;
}

static AVLNode *min_value_node(AVLNode *n) {
    AVLNode *cur = n;
    while (cur && cur->left) cur = cur->left;
    return cur;
}

static AVLNode *delete_rec(AVLNode *root, long long id, int *status) {
    if (!root) {
        *status = -1;
        return NULL;
    }

    if (id < root->player->id)
        root->left = delete_rec(root->left, id, status);
    else if (id > root->player->id)
        root->right = delete_rec(root->right, id, status);
    else {
        if (!root->left || !root->right) {
            AVLNode *temp = root->left ? root->left : root->right;
            free_node(root);
            *status = 0;
            return temp;
        }
        AVLNode *temp = min_value_node(root->right);
        player_free(root->player);
        root->player = player_clone(temp->player);
        root->right = delete_rec(root->right, temp->player->id, status);
    }

    if (*status != 0) return root;

    root->height = 1 + max_int(height(root->left), height(root->right));
    int balance = get_balance(root);

    if (balance > 1 && get_balance(root->left) >= 0)
        return rotate_right(root);
    if (balance > 1 && get_balance(root->left) < 0) {
        root->left = rotate_left(root->left);
        return rotate_right(root);
    }
    if (balance < -1 && get_balance(root->right) <= 0)
        return rotate_left(root);
    if (balance < -1 && get_balance(root->right) > 0) {
        root->right = rotate_right(root->right);
        return rotate_left(root);
    }
    return root;
}

int avl_delete(AVLTree *tree, long long id) {
    int status = 0;
    tree->root = delete_rec(tree->root, id, &status);
    if (status == 0) tree->size--;
    return status;
}

Player *avl_search(const AVLTree *tree, long long id) {
    AVLNode *cur = tree->root;
    while (cur) {
        if (id == cur->player->id)
            return cur->player;
        cur = (id < cur->player->id) ? cur->left : cur->right;
    }
    return NULL;
}

/* Exact rank via in-order traversal counting nodes before target. */
long long avl_rank_of(const AVLTree *tree, long long id) {
    long long rank = 0;
    AVLNode *cur = tree->root;
    while (cur) {
        int cmp = (id == cur->player->id) ? 0 : ((id < cur->player->id) ? 1 : -1);
        if (cmp == 0) {
            /* Count all nodes in left subtree + 1 */
            long long left_count = 0;
            AVLNode *stack[128];
            int top = 0;
            AVLNode *n = cur->left;
            while (n || top > 0) {
                while (n) {
                    stack[top++] = n;
                    n = n->left;
                }
                n = stack[--top];
                left_count++;
                n = n->right;
            }
            return left_count + 1;
        }
        if (cmp > 0) { /* id < cur->id, go left */
            cur = cur->left;
        } else { /* id > cur->id, go right: all left + cur are before */
            long long left_count = 0;
            AVLNode *stack[128];
            int top = 0;
            AVLNode *n = cur->left;
            while (n || top > 0) {
                while (n) {
                    stack[top++] = n;
                    n = n->left;
                }
                n = stack[--top];
                left_count++;
                n = n->right;
            }
            rank += left_count + 1;
            cur = cur->right;
        }
    }
    return 0;
}

static void collect_top(const AVLNode *root, long long count, Player *out[], long long *written) {
    if (!root || *written >= count) return;
    collect_top(root->left, count, out, written);
    if (*written < count) {
        out[(*written)++] = root->player;
    }
    collect_top(root->right, count, out, written);
}

long long avl_top(const AVLTree *tree, long long count, Player *out[]) {
    long long written = 0;
    collect_top(tree->root, count, out, &written);
    return written;
}

long long avl_size(const AVLTree *tree) {
    return tree ? tree->size : 0;
}

static void destroy_rec(AVLNode *n) {
    if (!n) return;
    destroy_rec(n->left);
    destroy_rec(n->right);
    free_node(n);
}

void avl_destroy(AVLTree *tree) {
    if (!tree) return;
    destroy_rec(tree->root);
    tree->root = NULL;
    tree->size = 0;
}

void avl_init(AVLTree *tree) {
    if (tree) {
        tree->root = NULL;
        tree->size = 0;
    }
}