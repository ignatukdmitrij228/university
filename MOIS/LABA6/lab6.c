#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

// ПРЕДСТАВЛЕНИЕ ДЕРЕВА
typedef struct {
    int left;
    int right;
} Node;

typedef struct {
    int n;              // число вершин 
    int root;           // корень 
    Node* nodes;        // массив вершин (индексы 1..n) 
} Tree;

// Инициализация дерева 
static void init_tree(Tree* t, int n) {
    t->n = n;
    t->root = 0;
    t->nodes = (Node*)calloc(n + 1, sizeof(Node));
}

// Освобождение 
static void free_tree(Tree* t) {
    if (t->nodes) {
        free(t->nodes);
        t->nodes = NULL;
    }
    t->n = 0;
    t->root = 0;
}

// Чтение дерева из файла 
static int read_tree(const char* fname, Tree* t) {
    FILE* f = fopen(fname, "r");
    if (!f) { printf("Oshibka\n"); return 0; }

    char line[256];
    int n = 0;
    int header_ok = 0;
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "%d", &n) == 1) { header_ok = 1; break; }
    }

    if (!header_ok || n <= 0) {
        printf("Oshibka\n");
        fclose(f);
        return 0;
    }

    init_tree(t, n);
    int first_mentioned = 0;

    while (fgets(line, sizeof(line), f)) {
        int u = 0, v = 0;
        int got = sscanf(line, "%d %d", &u, &v);
        if (got < 2) continue;

        // Проверки на корректность
        if (u < 1 || u > n || v < 1 || v > n || u == v) {
            printf("Oshibka\n");
            fclose(f);
            free_tree(t); // Освобождаем память при ошибке!
            return 0;
        }

        if (first_mentioned == 0) { first_mentioned = u; }

        // Вставка ребёнка
        if (t->nodes[u].left == 0) {
            t->nodes[u].left = v;
        }
        else if (t->nodes[u].right == 0) {
            t->nodes[u].right = v;
        }
        else {
            printf("Oshibka (bolshe 2 detey)\n");
            fclose(f);
            free_tree(t); // Освобождаем память при ошибке!
            return 0;
        }
    }
    fclose(f);

    t->root = first_mentioned ? first_mentioned : 1;
    return 1;
}

// ИТЕРАТИВНЫЕ ОБХОДЫ
static void preorder(const Tree* t, int* res, int* len) {
    int* stack = (int*)malloc((t->n + 1) * sizeof(int));
    int top = 0;
    *len = 0;
    if (t->root == 0) { free(stack); return; }
    stack[top++] = t->root;

    while (top > 0) {
        int u = stack[--top];
        res[(*len)++] = u;
        if (t->nodes[u].right) stack[top++] = t->nodes[u].right;
        if (t->nodes[u].left)  stack[top++] = t->nodes[u].left;
    }
    free(stack);
}

static void inorder(const Tree* t, int* res, int* len) {
    int* stack = (int*)malloc((t->n + 1) * sizeof(int));
    int top = 0;
    *len = 0;
    int curr = t->root;

    while (curr != 0 || top > 0) {
        while (curr != 0) {
            stack[top++] = curr;
            curr = t->nodes[curr].left;
        }
        curr = stack[--top];
        res[(*len)++] = curr;
        curr = t->nodes[curr].right;
    }
    free(stack);
}

static void postorder(const Tree* t, int* res, int* len) {
    int* stack1 = (int*)malloc((t->n + 1) * sizeof(int));
    int* stack2 = (int*)malloc((t->n + 1) * sizeof(int));
    int top1 = 0, top2 = 0;
    *len = 0;

    if (t->root == 0) { free(stack1); free(stack2); return; }
    stack1[top1++] = t->root;

    while (top1 > 0) {
        int u = stack1[--top1];
        stack2[top2++] = u;
        if (t->nodes[u].left)  stack1[top1++] = t->nodes[u].left;
        if (t->nodes[u].right) stack1[top1++] = t->nodes[u].right;
    }
    while (top2 > 0) {
        res[(*len)++] = stack2[--top2];
    }
    free(stack1); free(stack2);
}

// ПРОВЕРКА ИЗОМОРФИЗМА
static int are_isomorphic(const Tree* t1, const Tree* t2) {
    if (t1->n != t2->n) return 0;

    int* s1 = (int*)malloc((t1->n + 1) * sizeof(int));
    int* s2 = (int*)malloc((t1->n + 1) * sizeof(int));
    int top = 0;

    s1[top] = t1->root;
    s2[top] = t2->root;
    top++;

    int iso = 1;

    while (top > 0) {
        top--;
        int u1 = s1[top];
        int u2 = s2[top];

        if (u1 == 0 && u2 == 0) continue;
        if (u1 == 0 || u2 == 0) { iso = 0; break; }

        s1[top] = t1->nodes[u1].left;
        s2[top] = t2->nodes[u2].left;
        top++;

        s1[top] = t1->nodes[u1].right;
        s2[top] = t2->nodes[u2].right;
        top++;
    }

    free(s1); free(s2);
    return iso;
}

// ВЫСОТА И СБАЛАНСИРОВАННОСТЬ
static int tree_height(const Tree* t, int u) {
    if (u == 0) return 0;

    int max_h = 0;
    int* sv = (int*)malloc((t->n + 1) * sizeof(int));
    int* sd = (int*)malloc((t->n + 1) * sizeof(int));
    int top = 0;

    sv[top] = u;
    sd[top] = 1;
    top++;

    while (top > 0) {
        top--;
        int v = sv[top];
        int d = sd[top];

        if (d > max_h) max_h = d;

        if (t->nodes[v].left) { sv[top] = t->nodes[v].left; sd[top] = d + 1; top++; }
        if (t->nodes[v].right) { sv[top] = t->nodes[v].right; sd[top] = d + 1; top++; }
    }

    free(sv); free(sd);
    return max_h;
}

static int is_balanced(const Tree* t) {
    for (int i = 1; i <= t->n; i++) {
        int hl = tree_height(t, t->nodes[i].left);
        int hr = tree_height(t, t->nodes[i].right);
        int diff = hl - hr;
        if (diff < 0) diff = -diff;
        if (diff > 1) return 0;
    }
    return 1;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Oshibka (nuzhno 2 fayla)\n");
        return 1;
    }

    Tree t1, t2;
    if (!read_tree(argv[1], &t1)) return 1;

    // Если второе дерево не прочиталось, освобождаем первое
    if (!read_tree(argv[2], &t2)) { free_tree(&t1); return 1; }

    printf("=== Derevo 1 (%s) ===\n", argv[1]);
    printf("Vershin: %d, Koren: %d\n", t1.n, t1.root);

    int* res = (int*)malloc((t1.n + 1) * sizeof(int));
    int len;

    preorder(&t1, res, &len);
    printf("Pryamoy: "); for (int i = 0; i < len; i++) printf("%d ", res[i]); printf("\n");

    inorder(&t1, res, &len);
    printf("Simmetrichnyy: "); for (int i = 0; i < len; i++) printf("%d ", res[i]); printf("\n");

    postorder(&t1, res, &len);
    printf("Obratnyy: "); for (int i = 0; i < len; i++) printf("%d ", res[i]); printf("\n");

    int h1 = tree_height(&t1, t1.root);
    printf("Vysota: %d\n", h1);
    printf("Sbalansirovano: %s\n\n", is_balanced(&t1) ? "da" : "net");

    printf("=== Derevo 2 (%s) ===\n", argv[2]);
    printf("Vershin: %d, Koren: %d\n", t2.n, t2.root);

    preorder(&t2, res, &len);
    printf("Pryamoy: "); for (int i = 0; i < len; i++) printf("%d ", res[i]); printf("\n");

    inorder(&t2, res, &len);
    printf("Simmetrichnyy: "); for (int i = 0; i < len; i++) printf("%d ", res[i]); printf("\n");

    postorder(&t2, res, &len);
    printf("Obratnyy: "); for (int i = 0; i < len; i++) printf("%d ", res[i]); printf("\n");

    int h2 = tree_height(&t2, t2.root);
    printf("Vysota: %d\n", h2);
    printf("Sbalansirovano: %s\n\n", is_balanced(&t2) ? "da" : "net");

    printf("=== PROVERKA NA IZOMORFNOST ===\n");
    if (are_isomorphic(&t1, &t2)) {
        printf("Derevya izomorfny.\n");
    }
    else {
        printf("Derevya NE izomorfny.\n");
    }

    free(res);
    free_tree(&t1);
    free_tree(&t2);
    return 0;
}