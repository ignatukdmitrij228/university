#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

#define INF 1000000000000000000LL   // «бесконечность» 

// ПРЕДСТАВЛЕНИЕ ГРАФА: СПИСКИ СМЕЖНОСТИ (со весами)

typedef struct Edge {
    int v;              // соседняя вершина 
    long long w;        // вес ребра 
    struct Edge* next;
} Edge;

typedef struct {
    int   n;
    int   m;
    Edge** adj;
} Graph;

// Добавление неориентированного взвешенного ребра
/* direction = 1 — только u→v (направленное)
   direction = 0 — и u→v, и v→u (неориентированное) */
static void add_edge(Graph* g, int u, int v, long long w, int direction)
{
    Edge* p = (Edge*)malloc(sizeof(Edge));
    p->v = v; p->w = w; p->next = g->adj[u]; g->adj[u] = p;

    if (!direction) {
        p = (Edge*)malloc(sizeof(Edge));
        p->v = u; p->w = w; p->next = g->adj[v]; g->adj[v] = p;
    }

    g->m++;
}

// Чтение графа из файла
// Формат: первая строка — n; далее строки u v w 
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
        if (got < 3) continue;              // нужны все три числа

        if (u < 1 || u > g->n || v < 1 || v > g->n) {
            fprintf(stderr, "%s: rebro (%d,%d) vne [1..%d], propusk\n",
                fname, u, v, g->n);
            continue;
        }
        if (u == v) {
            fprintf(stderr, "%s: petlya (%d,%d), propusk\n", fname, u, v);
            continue;
        }
        // Отрицательные веса ДОПУСКАЮТСЯ — их обрабатывает Флойд.
        // Дейкстра будет пропущена (см. main).
        int direction = (w < 0) ? 1 : 0;   // отриц. вес → направленное
        add_edge(g, u, v, w, direction);
    }
    fclose(f);
    return 1;
}

// Освобождение
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

/* ПРОВЕРКА: есть ли в графе ребро с отрицательным весом.
   Возвращает 1, если хотя бы одно ребро отрицательно. */
static int has_negative_edges(const Graph* g)
{
    for (int i = 1; i <= g->n; i++)
        for (const Edge* e = g->adj[i]; e; e = e->next)
            if (e->w < 0) return 1;
    return 0;
}

/* АЛГОРИТМ ДЕЙКСТРЫ
   Находит кратчайшие расстояния от стартовой вершины s до всех.
   dist[i] — расстояние от s до i
   prev[i] — предшественник i на кратчайшем пути
   visited[i] — 1, если вершина уже «закрыта» */
static void dijkstra(const Graph* g, int s, long long* dist, int* prev)
{
    int N = g->n;
    int* visited = (int*)calloc(N + 1, sizeof(int));

    for (int i = 1; i <= N; i++) {
        dist[i] = INF;
        prev[i] = 0;
    }
    dist[s] = 0;

    for (int step = 0; step < N; step++) {
        // Выбираем непосещённую вершину с минимальным dist
        int u = -1;
        long long best = INF;
        for (int i = 1; i <= N; i++) {
            if (!visited[i] && dist[i] < best) {
                best = dist[i];
                u = i;
            }
        }
        if (u == -1) break;   // все оставшиеся недостижимы 

        visited[u] = 1;

        // Релаксация соседей
        for (const Edge* e = g->adj[u]; e; e = e->next) {
            int v = e->v;
            if (!visited[v] && dist[u] + e->w < dist[v]) {
                dist[v] = dist[u] + e->w;
                prev[v] = u;
            }
        }
    }

    free(visited);
}

// Восстановление пути s → t по массиву prev
static void print_path(int* prev, int s, int t)
{
    if (t == s) { printf("%d", s); return; }
    if (prev[t] == 0) { printf("(net puti)"); return; }

    // Рекурсивно не надо — идём назад и разворачиваем
    int stack[1024];
    int top = 0;
    int cur = t;
    while (cur != 0) {
        stack[top++] = cur;
        if (cur == s) break;
        cur = prev[cur];
    }
    for (int i = top - 1; i >= 0; i--) {
        printf("%d", stack[i]);
        if (i > 0) printf(" -> ");
    }
}

/* АЛГОРИТМ ФЛОЙДА–УОРШЕЛЛА
   d[i][j] — кратчайшее расстояние между i и j.
   next[i][j] — следующая вершина на пути i → j (для восстановления). */
static void floyd_warshall(const Graph* g, long long** d, int** next)
{
    int N = g->n;

    // Инициализация 
    for (int i = 1; i <= N; i++) {
        for (int j = 1; j <= N; j++) {
            if (i == j) d[i][j] = 0;
            else d[i][j] = INF;
            next[i][j] = 0;
        }
    }

    // Прямые рёбра 
    for (int u = 1; u <= N; u++) {
        for (const Edge* e = g->adj[u]; e; e = e->next) {
            int v = e->v;
            if (e->w < d[u][v]) {
                d[u][v] = e->w;
                next[u][v] = v;
            }
        }
    }

    // Тройной цикл 
    for (int k = 1; k <= N; k++) {
        for (int i = 1; i <= N; i++) {
            for (int j = 1; j <= N; j++) {
                if (d[i][k] < INF && d[k][j] < INF) {
                    long long through = d[i][k] + d[k][j];
                    if (through < d[i][j]) {
                        d[i][j] = through;
                        next[i][j] = next[i][k];
                    }
                }
            }
        }
    }
}

/* ПРОВЕРКА НА ОТРИЦАТЕЛЬНЫЙ ЦИКЛ.
   После работы Флойда, если на диагонали есть d[i][i] < 0,
   то через вершину i проходит отрицательный цикл.
   Возвращает 1, если цикл найден. */
static int has_negative_cycle(long long** d, int N)
{
    for (int i = 1; i <= N; i++)
        if (d[i][i] < 0) return 1;
    return 0;
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

    // АЛГОРИТМ ДЕЙКСТРЫ от вершины 1 — только если нет отрицательных рёбер
    printf("=== ALGORITM DEYKSTRY (ot vershiny 1) ===\n");

    if (has_negative_edges(&g)) {
        printf("V grafe est otricatelnye rebra.\n");
        printf("Algoritm Deykstry ne primenim — propuskaem.\n\n");
    }
    else {
        long long* dist = (long long*)malloc((N + 1) * sizeof(long long));
        int* prev = (int*)malloc((N + 1) * sizeof(int));
        if (!dist || !prev) {
            free(dist); free(prev); free_graph(&g); return 1;
        }

        dijkstra(&g, 1, dist, prev);

        printf("Krachaishie rasstoyaniya ot 1:\n");
        for (int i = 1; i <= N; i++) {
            printf("  do %2d: ", i);
            if (dist[i] >= INF) printf("nedostizhima\n");
            else printf("%lld\n", dist[i]);
        }

        printf("\nKrachaishie puti ot 1:\n");
        for (int i = 1; i <= N; i++) {
            if (i == 1) continue;
            printf("  do %2d: ", i);
            if (dist[i] >= INF) printf("(net puti)\n");
            else { print_path(prev, 1, i); printf("   (dlina %lld)\n", dist[i]); }
        }

        free(dist);
        free(prev);
        printf("\n");
    }

    //  АЛГОРИТМ ФЛОЙДА–УОРШЕЛЛА — работает всегда
    printf("=== ALGORITM FLOYD–WARSHALL ===\n");

    long long** d = (long long**)malloc((N + 1) * sizeof(long long*));
    int** nx = (int**)malloc((N + 1) * sizeof(int*));
    for (int i = 0; i <= N; i++) {
        d[i] = (long long*)malloc((N + 1) * sizeof(long long));
        nx[i] = (int*)malloc((N + 1) * sizeof(int));
    }

    floyd_warshall(&g, d, nx);

    // Проверка на отрицательный цикл
    if (has_negative_cycle(d, N)) {
        printf("V grafe obnaruzhen otricatelnyy cikl!\n");
        printf("Krachaishie rasstoyaniya ne opredeleny (stremyatsya k -inf).\n");
    }
    else {
        printf("Matritsa krachaishih rasstoyaniy:\n     ");
        for (int j = 1; j <= N; j++) printf("%15d", j);
        printf("\n");
        for (int i = 1; i <= N; i++) {
            printf("%4d:", i);
            for (int j = 1; j <= N; j++) {
                if (d[i][j] >= INF) printf("%15s", "INF");
                else printf("%15lld", d[i][j]);
            }
            printf("\n");
        }

        // Кратчайшие пути от 1 до всех по Флойду 
        printf("\nKrachaishie rasstoyaniya ot 1 (Floyd):\n");
        for (int j = 1; j <= N; j++) {
            printf("  do %2d: ", j);
            if (d[1][j] >= INF) printf("nedostizhima\n");
            else printf("%lld\n", d[1][j]);
        }
    }

    for (int i = 0; i <= N; i++) { free(d[i]); free(nx[i]); }
    free(d);
    free(nx);

    free_graph(&g);
    return 0;
}