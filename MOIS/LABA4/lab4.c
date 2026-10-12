#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

// ПРЕДСТАВЛЕНИЕ ГРАФА: СПИСКИ СМЕЖНОСТИ (со весами)
typedef struct Edge {
    int v;
    long long w;
    struct Edge* next;
} Edge;

typedef struct {
    int   n;
    int   m;
    Edge** adj;
} Graph;

static void add_edge(Graph* g, int u, int v, long long w)
{
    Edge* p = (Edge*)malloc(sizeof(Edge));
    p->v = v; p->w = w; p->next = g->adj[u]; g->adj[u] = p;

    p = (Edge*)malloc(sizeof(Edge));
    p->v = u; p->w = w; p->next = g->adj[v]; g->adj[v] = p;

    g->m++;
}

static int read_graph(const char* fname, Graph* g)
{
    FILE* f = fopen(fname, "r");
    if (!f) { fprintf(stderr, "ne otkryt %s\n", fname); return 0; }

    char line[256];
    int n = 0;
    int header_ok = 0;
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "%d", &n) == 1) { header_ok = 1; break; }
    }
    if (!header_ok || n < 0) {
        fprintf(stderr, "%s: plokhoye n\n", fname);
        fclose(f); return 0;
    }

    g->n = n;
    g->m = 0;
    g->adj = (Edge**)calloc(n + 1, sizeof(Edge*));
    if (!g->adj) { fclose(f); return 0; }

    while (fgets(line, sizeof(line), f)) {
        int u = 0, v = 0;
        long long w = 0;
        int got = sscanf(line, "%d %d %lld", &u, &v, &w);
        if (got < 3) continue;

        if (u < 1 || u > g->n || v < 1 || v > g->n) {
            fprintf(stderr, "%s: rebro (%d,%d) vne [1..%d], propusk\n",
                fname, u, v, g->n);
            continue;
        }
        if (u == v) {
            fprintf(stderr, "%s: petlya (%d,%d), propusk\n", fname, u, v);
            continue;
        }
        if (w <= 0) {
            fprintf(stderr, "%s: nepolozhitelnyy ves (%d,%d,%lld), propusk\n",
                fname, u, v, w);
            continue;
        }
        add_edge(g, u, v, w);
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

// список рёбер + проверка связности
typedef struct {
    int u, v;
    long long w;
} WEdge;

// Собираем рёбра один раз (u < v) 
static int collect_edges(const Graph* g, WEdge* edges)
{
    int idx = 0;
    for (int u = 1; u <= g->n; u++)
        for (const Edge* e = g->adj[u]; e; e = e->next)
            if (u < e->v) {
                edges[idx].u = u;
                edges[idx].v = e->v;
                edges[idx].w = e->w;
                idx++;
            }
    return idx;
}

// Проверка связности обходом (итеративный DFS)
static int is_connected(const Graph* g, int* visited, int* stack)
{
    if (g->n == 0) return 1;
    for (int i = 1; i <= g->n; i++) visited[i] = 0;

    int top = 0;
    stack[top++] = 1;
    visited[1] = 1;
    while (top > 0) {
        int u = stack[--top];
        for (const Edge* e = g->adj[u]; e; e = e->next)
            if (!visited[e->v]) { visited[e->v] = 1; stack[top++] = e->v; }
    }

    for (int i = 1; i <= g->n; i++)
        if (!visited[i]) return 0;
    return 1;
}

/*  АЛГОРИТМ ПРИМА
 *  Идея: растим дерево из вершины 1.
 *  На каждом шаге добавляем вершину с минимальным ребром,
 *  ведущим из дерева наружу. */
static long long prim(const Graph* g, WEdge* result)
{
    int N = g->n;
    int* inTree = (int*)calloc(N + 1, sizeof(int));  // вершина уже в дереве?
    long long* key = (long long*)malloc((N + 1) * sizeof(long long)); // мин. вес ребра
    int* parent = (int*)malloc((N + 1) * sizeof(int));

    const long long INF = 1000000000000000000LL;

    for (int i = 1; i <= N; i++) { key[i] = INF; parent[i] = 0; }

    key[1] = 0;  // стартовая вершина 
    long long total = 0;
    int cnt = 0; // сколько рёбер добавили

    for (int step = 0; step < N; step++) {
        // Ищем непосещённую вершину с минимальным key 
        int u = -1;
        long long best = INF;
        for (int i = 1; i <= N; i++) {
            if (!inTree[i] && key[i] < best) { best = key[i]; u = i; }
        }
        if (u == -1) break;
        inTree[u] = 1;

        if (parent[u] != 0) {
            result[cnt].u = parent[u];
            result[cnt].v = u;
            result[cnt].w = key[u];
            cnt++;
            total += key[u];
        }

        // Обновляем соседей 
        for (const Edge* e = g->adj[u]; e; e = e->next) {
            int v = e->v;
            if (!inTree[v] && e->w < key[v]) {
                key[v] = e->w;
                parent[v] = u;
            }
        }
    }

    free(inTree); free(key); free(parent);
    return total;
}

/*  АЛГОРИТМ КРАСКАЛА
 *  Идея: сортируем рёбра по весу, добавляем в дерево,
 *  если они соединяют разные компоненты (через СНМ). */

 // СНМ — система непересекающихся множеств
static int* dsu_parent;

static int dsu_find(int x)
{
    while (dsu_parent[x] != x) {
        dsu_parent[x] = dsu_parent[dsu_parent[x]];  /* сжатие пути */
        x = dsu_parent[x];
    }
    return x;
}

static void dsu_union(int a, int b)
{
    a = dsu_find(a);
    b = dsu_find(b);
    if (a != b) dsu_parent[a] = b;
}

// Сравнение рёбер по весу — для qsort 
static int cmp_edge(const void* a, const void* b)
{
    long long wa = ((const WEdge*)a)->w;
    long long wb = ((const WEdge*)b)->w;
    if (wa < wb) return -1;
    if (wa > wb) return 1;
    return 0;
}

static long long kruskal(const Graph* g, WEdge* all, int m, WEdge* result)
{
    int N = g->n;

    // Сортируем рёбра
    qsort(all, m, sizeof(WEdge), cmp_edge);

    // СНМ: каждая вершина — своё множество 
    dsu_parent = (int*)malloc((N + 1) * sizeof(int));
    for (int i = 1; i <= N; i++) dsu_parent[i] = i;

    long long total = 0;
    int cnt = 0;

    for (int i = 0; i < m; i++) {
        int u = all[i].u;
        int v = all[i].v;
        if (dsu_find(u) != dsu_find(v)) {
            dsu_union(u, v);
            result[cnt++] = all[i];
            total += all[i].w;
            if (cnt == N - 1) break;  // дерево построено
        }
    }

    free(dsu_parent);
    return total;
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
    printf("Vershin: %d, ryober: %d\n\n", g.n, g.m);

    int N = g.n;

    int* visited = (int*)malloc((N + 1) * sizeof(int));
    int* buf = (int*)malloc((N + 1) * sizeof(int));
    if (!visited || !buf) {
        free(visited); free(buf); free_graph(&g); return 1;
    }

    // Проверка связности — МОД существует только для связного графа
    if (!is_connected(&g, visited, buf)) {
        printf("Graf nesvyaznyy — minimalnogo ostovnogo dereva net.\n");
        free(visited); free(buf); free_graph(&g);
        return 0;
    }
    printf("Graf svyaznyy.\n\n");

    WEdge* result_prim = (WEdge*)malloc((N) * sizeof(WEdge));
    WEdge* result_kruskal = (WEdge*)malloc((N) * sizeof(WEdge));
    WEdge* all_edges = (WEdge*)malloc((g.m + 1) * sizeof(WEdge));
    if (!result_prim || !result_kruskal || !all_edges) {
        free(visited); free(buf);
        free(result_prim); free(result_kruskal); free(all_edges);
        free_graph(&g); return 1;
    }

    int m = collect_edges(&g, all_edges);

    // ПРИМ
    printf("=== ALGORITM PRIMA ===\n");
    long long total_prim = prim(&g, result_prim);
    printf("Reobra MOD (%d shtuk):\n", N - 1);
    for (int i = 0; i < N - 1; i++) {
        printf("  %d - %d   ves %lld\n",
            result_prim[i].u, result_prim[i].v, result_prim[i].w);
    }
    printf("Summarnyy ves: %lld\n\n", total_prim);

    // КРАСКАЛ
    printf("=== ALGORITM KRASKALA ===\n");
    long long total_kruskal = kruskal(&g, all_edges, m, result_kruskal);
    printf("Reobra MOD (%d shtuk):\n", N - 1);
    for (int i = 0; i < N - 1; i++) {
        printf("  %d - %d   ves %lld\n",
            result_kruskal[i].u, result_kruskal[i].v, result_kruskal[i].w);
    }
    printf("Summarnyy ves: %lld\n\n", total_kruskal);

    // Сверка
    if (total_prim != total_kruskal) {
        printf("VNIMANIYE: rezultaty Prima i Kraskala ne sovpali!\n");
    }
    else {
        printf("Rezultaty sovpali: ves MOD = %lld\n", total_prim);
    }

    free(result_prim); free(result_kruskal); free(all_edges);
    free(visited); free(buf);
    free_graph(&g);
    return 0;
}