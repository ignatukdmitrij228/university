#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

// "Бесконечность" — большое число, влезает в long long при сложении 
#define INF 1000000000000000000LL

// ПРЕДСТАВЛЕНИЕ ГРАФА: СПИСКИ СМЕЖНОСТИ (со весами)
typedef struct Edge {
    int v;              // соседняя вершина 
    long long w;        // вес ребра 
    struct Edge* next;  // следующий сосед в списке 
} Edge;

typedef struct {
    int   n;            // число вершин 
    int   m;            // число рёбер
    Edge** adj;         // adj[i] — голова списка соседей вершины i 
} Graph;

// Добавление неориентированного взвешенного ребра
static void add_edge(Graph* g, int u, int v, long long w)
{
    // Добавляем ребро в список u: сосед — v 
    Edge* p = (Edge*)malloc(sizeof(Edge));
    p->v = v; p->w = w; p->next = g->adj[u]; g->adj[u] = p;

    // Добавляем зеркально в список v: сосед — u 
    p = (Edge*)malloc(sizeof(Edge));
    p->v = u; p->w = w; p->next = g->adj[v]; g->adj[v] = p;

    g->m++;
}

// Чтение графа из файла 
// Формат: первая строка — n; далее строки u v w 
static int read_graph(const char* fname, Graph* g)
{
    FILE* f = fopen(fname, "r");
    if (!f) { printf("Oshibka\n"); return 0; }

    char line[256];
    int n = 0;
    int header_ok = 0;

    // Читаем первую непустую строку — число вершин 
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "%d", &n) == 1) { header_ok = 1; break; }
    }
    if (!header_ok || n < 0) {
        printf("Oshibka\n");
        fclose(f); return 0;
    }

    g->n = n;
    g->m = 0;
    g->adj = (Edge**)calloc(n + 1, sizeof(Edge*));   // все NULL 
    if (!g->adj) { printf("Oshibka\n"); fclose(f); return 0; }

    // Читаем рёбра: u v w 
    while (fgets(line, sizeof(line), f)) {
        int u = 0, v = 0;
        long long w = 0;
        int got = sscanf(line, "%d %d %lld", &u, &v, &w);
        if (got < 3) continue;              // пустая строка или мусор 

        // Вершина вне диапазона [1..n] — ошибка 
        if (u < 1 || u > g->n || v < 1 || v > g->n) {
            printf("Oshibka\n");
            fclose(f); return 0;
        }
        if (w < 0) {
            printf("Oshibka\n");
            fclose(f); return 0;
        }
        // Петля — пропускаем (не влияет на кратчайшие пути) 
        if (u == v) continue;

        add_edge(g, u, v, w);
    }
    fclose(f);
    return 1;
}

// Освобождение памяти 
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

/*  АЛГОРИТМ ДЕЙКСТРЫ
 *  Ищет кратчайшие расстояния от вершины s до всех остальных.
 *  Работает только при неотрицательных весах.
 *  dist[i] — кратчайшее расстояние от s до i
 *  prev[i] — предшественник i на кратчайшем пути (для восстановления)
 *  visited[i] — 1, если вершина уже «зафиксирована» */
static void dijkstra(const Graph* g, int s, long long* dist, int* prev)
{
    int N = g->n;
    int* visited = (int*)calloc(N + 1, sizeof(int));

    // Инициализация
    for (int i = 1; i <= N; i++) { dist[i] = INF; prev[i] = 0; }
    dist[s] = 0;

    // N итераций — на каждой фиксируем одну вершину 
    for (int step = 0; step < N; step++) {

        // 1) Находим непосещённую вершину с минимальным dist 
        int u = -1;
        long long best = INF;
        for (int i = 1; i <= N; i++) {
            if (!visited[i] && dist[i] < best) { best = dist[i]; u = i; }
        }
        if (u == -1) break;   // оставшиеся недостижимы

        // 2) Фиксируем её 
        visited[u] = 1;

        // 3) Обновляем расстояния соседей 
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

    // Идём назад от t к s, складывая вершины в стек 
    int stack[1024];
    int top = 0;
    int cur = t;
    while (cur != 0) {
        stack[top++] = cur;
        if (cur == s) break;
        cur = prev[cur];
    }
    // Выводим в обратном порядке 
    for (int i = top - 1; i >= 0; i--) {
        printf("%d", stack[i]);
        if (i > 0) printf(" -> ");
    }
}

/*  АЛГОРИТМ ФЛОЙДА–УОРШЕЛЛА
 *  Ищет кратчайшие расстояния между ВСЕМИ парами вершин.
 *  Работает и с отрицательными весами (если нет отрицательных циклов).
 *  d[i][j] — кратчайшее расстояние от i до j */
static void floyd_warshall(const Graph* g, long long** d)
{
    int N = g->n;

    // Инициализация: 0 на диагонали, ∞ вне, веса прямых рёбер
    for (int i = 1; i <= N; i++)
        for (int j = 1; j <= N; j++)
            d[i][j] = (i == j) ? 0 : INF;

    // Прямые рёбра — берём минимальный вес (для кратных)
    for (int u = 1; u <= N; u++)
        for (const Edge* e = g->adj[u]; e; e = e->next)
            if (e->w < d[u][e->v]) d[u][e->v] = e->w;

    // Тройной цикл — разрешаем вершину k как промежуточную
    for (int k = 1; k <= N; k++)
        for (int i = 1; i <= N; i++)
            for (int j = 1; j <= N; j++)
                if (d[i][k] < INF && d[k][j] < INF) {
                    long long through = d[i][k] + d[k][j];
                    if (through < d[i][j]) d[i][j] = through;
                }
}

int main(int argc, char** argv)
{
    if (argc < 2) { printf("Oshibka\n"); return 1; }

    Graph g;
    if (!read_graph(argv[1], &g)) return 1;

    printf("Fayl: %s\n", argv[1]);
    printf("Vershin: %d, ryober: %d\n\n", g.n, g.m);

    int N = g.n;

    // ДЕЙКСТРА от вершины 1
    printf("=== ALGORITM DEYKSTRY (ot vershiny 1) ===\n");

    long long* dist = (long long*)malloc((N + 1) * sizeof(long long));
    int* prev = (int*)malloc((N + 1) * sizeof(int));
    if (!dist || !prev) {
        printf("Oshibka\n");
        free(dist); free(prev); free_graph(&g); return 1;
    }

    dijkstra(&g, 1, dist, prev);

    // Вывод расстояний 
    printf("Krachaishie rasstoyaniya ot 1:\n");
    for (int i = 1; i <= N; i++) {
        printf("  do %2d: ", i);
        if (dist[i] >= INF) printf("nedostizhima\n");
        else printf("%lld\n", dist[i]);
    }

    // Вывод путей
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

    // ФЛОЙД–УОРШЕЛЛ
    printf("=== ALGORITM FLOYD-WARSHALL ===\n");

    long long** d = (long long**)malloc((N + 1) * sizeof(long long*));
    for (int i = 0; i <= N; i++)
        d[i] = (long long*)malloc((N + 1) * sizeof(long long));

    floyd_warshall(&g, d);

    // Вывод матрицы
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

    // Расстояния от 1 (для сверки с Дейкстрой)
    printf("\nKrachaishie rasstoyaniya ot 1 (Floyd):\n");
    for (int j = 1; j <= N; j++) {
        printf("  do %2d: ", j);
        if (d[1][j] >= INF) printf("nedostizhima\n");
        else printf("%lld\n", d[1][j]);
    }

    for (int i = 0; i <= N; i++) free(d[i]);
    free(d);
    free_graph(&g);
    return 0;
}