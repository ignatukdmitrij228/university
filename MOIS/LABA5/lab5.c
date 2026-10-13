#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

// ПРЕДСТАВЛЕНИЕ ГРАФА: СПИСКИ СМЕЖНОСТИ
typedef struct Edge {
    int v;
    int id;             // номер ребра в файле (для неориентированного — один на пару) 
    struct Edge* next;
} Edge;

typedef struct {
    int   n;
    int   m;
    int   directed;     // 0 — неориентированный, 1 — ориентированный 
    Edge** adj;
} Graph;

// Добавление ребра/дуги 
static void add_edge(Graph* g, int u, int v, int id)
{
    Edge* p = (Edge*)malloc(sizeof(Edge));
    p->v = v; p->id = id; p->next = g->adj[u]; g->adj[u] = p;

    if (!g->directed) {
        p = (Edge*)malloc(sizeof(Edge));
        p->v = u; p->id = id; p->next = g->adj[v]; g->adj[v] = p;
    }
}

// Чтение графа 
static int read_graph(const char* fname, Graph* g)
{
    FILE* f = fopen(fname, "r");
    if (!f) { fprintf(stderr, "ne otkryt %s\n", fname); return 0; }

    char line[256];
    int directed = 0, n = 0;
    int header_ok = 0;

    // Первая непустая строка — флаг directed 
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "%d", &directed) == 1) { header_ok = 1; break; }
    }
    if (!header_ok) { fprintf(stderr, "%s: net flaga\n", fname); fclose(f); return 0; }

    // Вторая непустая — n 
    header_ok = 0;
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "%d", &n) == 1) { header_ok = 1; break; }
    }
    if (!header_ok || n < 0) { fprintf(stderr, "%s: plokhoye n\n", fname); fclose(f); return 0; }

    g->n = n;
    g->m = 0;
    g->directed = directed ? 1 : 0;
    g->adj = (Edge**)calloc(n + 1, sizeof(Edge*));
    if (!g->adj) { fclose(f); return 0; }

    int edge_id = 0;
    while (fgets(line, sizeof(line), f)) {
        int u = 0, v = 0;
        int got = sscanf(line, "%d %d", &u, &v);
        if (got < 2) continue;

        if (u < 1 || u > g->n || v < 1 || v > g->n) {
            fprintf(stderr, "%s: rebro (%d,%d) vne [1..%d], propusk\n",
                fname, u, v, g->n);
            continue;
        }
        if (u == v) {
            fprintf(stderr, "%s: petlya (%d,%d), propusk\n", fname, u, v);
            continue;
        }
        edge_id++;
        add_edge(g, u, v, edge_id);
        g->m++;
    }
    fclose(f);
    return 1;
}

static void free_graph(Graph* g)
{
    for (int i = 1; i <= g->n; i++) {
        Edge* e = g->adj[i];
        while (e) { Edge* nx = e->next; free(e); e = nx; }
    }
    free(g->adj);
    g->adj = NULL;
    g->n = g->m = 0;
}

/*  ЗАДАЧА 1: КОМПОНЕНТЫ СИЛЬНОЙ СВЯЗНОСТИ (алгоритм Тарьяна)
 *  Итеративный, без рекурсии.
 *  tin[v]   — время входа в v
 *  low[v]   — минимальное tin, достижимое из v
 *  in_stack — v лежит в стеке
 *  stack[]  — стек вершин текущей компоненты */
typedef struct {
    int v;
    Edge* iter;   // указатель на текущую дугу в adj[v] 
} DFSFrame;

static void tarjan_scc(const Graph* g, int* comp)
{
    int N = g->n;
    int* tin = (int*)calloc(N + 1, sizeof(int));
    int* low = (int*)calloc(N + 1, sizeof(int));
    int* in_stack = (int*)calloc(N + 1, sizeof(int));
    int* stack = (int*)malloc((N + 1) * sizeof(int));
    DFSFrame* frames = (DFSFrame*)malloc((N + 1) * sizeof(DFSFrame));

    int time = 0;
    int top = 0;
    int comp_cnt = 0;
    for (int i = 1; i <= N; i++) comp[i] = 0;

    for (int start = 1; start <= N; start++) {
        if (tin[start]) continue;

        // Запуск DFS из start 
        int fsp = 0;
        frames[fsp].v = start;
        frames[fsp].iter = g->adj[start];
        fsp++;

        tin[start] = low[start] = ++time;
        stack[top++] = start;
        in_stack[start] = 1;

        while (fsp > 0) {
            DFSFrame* fr = &frames[fsp - 1];
            int v = fr->v;
            Edge* e = fr->iter;

            if (e != NULL) {
                fr->iter = e->next;
                int u = e->v;

                if (!tin[u]) {
                    // Идём в новую вершину 
                    tin[u] = low[u] = ++time;
                    stack[top++] = u;
                    in_stack[u] = 1;

                    frames[fsp].v = u;
                    frames[fsp].iter = g->adj[u];
                    fsp++;
                }
                else if (in_stack[u]) {
                    // Обратная дуга в стек — обновляем low 
                    if (tin[u] < low[v]) low[v] = tin[u];
                }
            }
            else {
                // Все дуги обработаны — снимаем кадр 
                fsp--;
                if (fsp > 0) {
                    int parent = frames[fsp - 1].v;
                    if (low[v] < low[parent]) low[parent] = low[v];
                }

                // v — корень компоненты? 
                if (tin[v] == low[v]) {
                    comp_cnt++;
                    while (top > 0) {
                        int w = stack[--top];
                        in_stack[w] = 0;
                        comp[w] = comp_cnt;
                        if (w == v) break;
                    }
                }
            }
        }
    }

    printf("Komponent silnoy svyaznosti: %d\n", comp_cnt);
    for (int c = 1; c <= comp_cnt; c++) {
        printf("  KSS %d: ", c);
        for (int i = 1; i <= N; i++) if (comp[i] == c) printf("%d ", i);
        printf("\n");
    }

    free(tin); free(low); free(in_stack); free(stack); free(frames);
}

/*  ЗАДАЧА 2: ТОЧКИ СОЧЛЕНЕНИЯ, МОСТЫ, БЛОКИ
 *  Для НЕориентированного графа.
 *  tin/low как в Тарьяне, но нельзя возвращаться по тому же ребру — храним ID ребра.
 *  Храним в кадре: v, iter (указатель в adj[v]), parent_edge_id
 *  Для определения моста: low[u] > tin[v] → мост (v,u)
 *  Точка сочленения: low[u] >= tin[v] (не корень), либо корень с 2+ детьми */

typedef struct {
    int v;
    Edge* iter;
    int parent_edge_id;
    int parent_v;
} DFSFrame2;

static void biconnected(const Graph* g)
{
    int N = g->n;
    int* tin = (int*)calloc(N + 1, sizeof(int));
    int* low = (int*)calloc(N + 1, sizeof(int));
    int* is_art = (int*)calloc(N + 1, sizeof(int));   // точка сочленения 
    int* is_bridge = (int*)calloc(g->m + 1, sizeof(int));  // мост по id 

    DFSFrame2* frames = (DFSFrame2*)malloc((N + 1) * sizeof(DFSFrame2));
    int* child_count = (int*)calloc(N + 1, sizeof(int));

    int time = 0;

    for (int start = 1; start <= N; start++) {
        if (tin[start]) continue;

        int fsp = 0;
        frames[fsp].v = start;
        frames[fsp].iter = g->adj[start];
        frames[fsp].parent_edge_id = -1;
        frames[fsp].parent_v = 0;
        fsp++;

        tin[start] = low[start] = ++time;

        while (fsp > 0) {
            DFSFrame2* fr = &frames[fsp - 1];
            int v = fr->v;
            Edge* e = fr->iter;

            if (e != NULL) {
                fr->iter = e->next;
                int u = e->v;

                // Не возвращаемся по тому же ребру 
                if (e->id == fr->parent_edge_id) continue;

                if (!tin[u]) {
                    // Ребёнок 
                    child_count[v]++;

                    tin[u] = low[u] = ++time;
                    frames[fsp].v = u;
                    frames[fsp].iter = g->adj[u];
                    frames[fsp].parent_edge_id = e->id;
                    frames[fsp].parent_v = v;
                    fsp++;
                }
                else {
                    // Обратное ребро (в предка) 
                    if (tin[u] < low[v]) low[v] = tin[u];
                }
            }
            else {
                // Снимаем кадр 
                fsp--;

                if (fsp > 0) {
                    DFSFrame2* parent = &frames[fsp - 1];
                    int pv = parent->v;
                    int peid = frames[fsp].parent_edge_id;

                    // low[pv] = min(low[pv], low[v]) 
                    if (low[v] < low[pv]) low[pv] = low[v];

                    // Мост? 
                    if (low[v] > tin[pv]) {
                        is_bridge[peid] = 1;
                    }

                    // Точка сочленения (не корень)? 
                    if (low[v] >= tin[pv] && pv != start) {
                        is_art[pv] = 1;
                    }
                    else if (low[v] >= tin[pv] && pv == start && child_count[start] > 1) {
                        is_art[pv] = 1;
                    }
                }
            }
        }
    }

    // Вывод 
    printf("\nTochki sochleneniya:\n  ");
    int cnt_art = 0;
    for (int i = 1; i <= N; i++) if (is_art[i]) { printf("%d ", i); cnt_art++; }
    if (cnt_art == 0) printf("net");
    printf("\n");

    printf("Mosty:\n  ");
    int cnt_bridge = 0;
    for (int u = 1; u <= N; u++) {
        for (const Edge* e = g->adj[u]; e; e = e->next) {
            if (e->id == 0) continue;
            if (is_bridge[e->id] && u < e->v) {
                printf("%d-%d ", u, e->v);
                cnt_bridge++;
            }
        }
    }
    if (cnt_bridge == 0) printf("net");
    printf("\n");

    free(tin); free(low); free(is_art); free(is_bridge);
    free(frames); free(child_count);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <graph-file>\n", argv[0]);
        return 1;
    }

    Graph g;
    if (!read_graph(argv[1], &g)) return 1;

    printf("Fayl: %s\n", argv[1]);
    printf("Tip: %s\n", g.directed ? "orientirovannyy" : "neorientirovannyy");
    printf("Vershin: %d, reber/dug: %d\n\n", g.n, g.m);

    if (g.directed) {
        //КСС 
        printf("=== KOMPONENTY SILNOY SVYAZNOSTI (Tarjan) ===\n");
        int* comp = (int*)malloc((g.n + 1) * sizeof(int));
        tarjan_scc(&g, comp);
        free(comp);
    }
    else {
        // Двусвязность
        printf("=== TOCHKI SOCHLENENIYA I MOSTY ===\n");
        biconnected(&g);
    }

    free_graph(&g);
    return 0;
}