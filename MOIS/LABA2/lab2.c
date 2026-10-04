#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

// ѕ–≈ƒ—“ј¬Ћ≈Ќ»≈ √–ј‘ј: —ѕ»— » —ћ≈∆Ќќ—“»
typedef struct Edge {
    int v;
    struct Edge* next;
} Edge;

typedef struct {
    int   n;
    int   m;
    Edge** adj;
} Graph;

// ƒобавление неориентированного ребра
static void add_edge(Graph* g, int u, int v)
{
    Edge* p = (Edge*)malloc(sizeof(Edge));
    p->v = v; p->next = g->adj[u]; g->adj[u] = p;

    p = (Edge*)malloc(sizeof(Edge));
    p->v = u; p->next = g->adj[v]; g->adj[v] = p;

    g->m++;
}

// „тение графа из файла
// ‘ормат: перва€ строка Ч n; далее строки u v 
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
        add_edge(g, u, v);
    }
    fclose(f);
    return 1;
}

// ќсвобождение 
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

// ¬—ѕќћќ√ј“≈Ћ№Ќџ≈ 
// —тепень вершины 
static int degree(const Graph* g, int u)
{
    int d = 0;
    for (const Edge* e = g->adj[u]; e; e = e->next) d++;
    return d;
}

// —межны ли u и v 
static int is_adj(const Graph* g, int u, int v)
{
    for (const Edge* e = g->adj[u]; e; e = e->next)
        if (e->v == v) return 1;
    return 0;
}

// ѕ–ќ¬≈– ј —¬я«Ќќ—“» Ч нужна дл€ критери€ Ёйлера
// »золированные вершины игнорируютс€.
static int is_connected(const Graph* g, int* visited, int* stack)
{
    if (g->n == 0) return 1;
    for (int i = 1; i <= g->n; i++) visited[i] = 0;

    int start = 1;
    for (int i = 1; i <= g->n; i++) if (g->adj[i]) { start = i; break; }

    int top = 0;
    stack[top++] = start;
    visited[start] = 1;
    while (top > 0) {
        int u = stack[--top];
        for (const Edge* e = g->adj[u]; e; e = e->next)
            if (!visited[e->v]) { visited[e->v] = 1; stack[top++] = e->v; }
    }

    for (int i = 1; i <= g->n; i++)
        if (g->adj[i] && !visited[i]) return 0;
    return 1;
}

// Ё…Ћ≈–ќ¬ ÷» Ћ
//  ритерий: св€зный + все степени чЄтные
static int has_euler_circuit(const Graph* g, int* visited, int* stack)
{
    if (g->n == 0 || g->m == 0) return 0;
    if (!is_connected(g, visited, stack)) return 0;
    for (int i = 1; i <= g->n; i++)
        if (degree(g, i) % 2 != 0) return 0;
    return 1;
}

// Ќахождение эйлерова цикла Ч алгоритм ’ирхольцера (итеративный).
// ¬озвращает длину цикла (число вершин в path), либо 0. 
static int euler_circuit(const Graph* g, int* path)
{
    if (g->m == 0) return 0;

    int N = g->n, M = g->m;

    // —оберЄм рЄбра один раз
    int* eu = (int*)malloc((M + 1) * sizeof(int));
    int* ev = (int*)malloc((M + 1) * sizeof(int));
    int idx = 0;
    for (int u = 1; u <= N; u++)
        for (const Edge* e = g->adj[u]; e; e = e->next)
            if (u < e->v) { idx++; eu[idx] = u; ev[idx] = e->v; }

    int* used = (int*)calloc(M + 1, sizeof(int));
    int* stack = (int*)malloc((2 * M + 2) * sizeof(int));
    int* cycle = (int*)malloc((M + 1) * sizeof(int));
    int top = 0, clen = 0;

    stack[top++] = 1;

    while (top > 0) {
        int u = stack[top - 1];
        int found = -1;
        for (int e = 1; e <= M; e++) {
            if (used[e]) continue;
            if (eu[e] == u || ev[e] == u) { found = e; break; }
        }
        if (found == -1) {
            top--;
            cycle[clen++] = u;
        }
        else {
            used[found] = 1;
            int nxt = (eu[found] == u) ? ev[found] : eu[found];
            stack[top++] = nxt;
        }
    }

    // –азворачиваем 
    for (int i = 0; i < clen / 2; i++) {
        int t = cycle[i]; cycle[i] = cycle[clen - 1 - i]; cycle[clen - 1 - i] = t;
    }
    for (int i = 0; i < clen; i++) path[i] = cycle[i];

    free(eu); free(ev); free(used); free(stack); free(cycle);
    return clen;
}

/*   √јћ»Ћ№“ќЌќ¬ ÷» Ћ Ч итеративный перебор с возвратом
    Ѕез рекурсии: свой стек откатов.
    »де€:
     - path[0..pos-1] Ч текущий путь;
     - used[] Ч какие вершины в пути;
     - iter[u] Ч указатель на "текущего соседа" в списке adj[u].
      огда возвращаемс€ к u и берЄм следующего соседа Ч iter[u]++.
     —тек "возвратов": в нЄм вершины, из которых мы ушли вглубь.
      огда путь дошЄл до длины n и последн€€ смежна с первой Ч успех. */
static int hamiltonian_circuit(const Graph* g, int* path)
{
    if (g->n < 3) return 0;

    int N = g->n;
    int* used = (int*)calloc(N + 1, sizeof(int));

    /* iter[u] Ч указатель на текущий узел списка adj[u].
       ’раним как указатель на Edge* Ч так не надо пересчитывать. */
    Edge** iter = (Edge**)calloc(N + 1, sizeof(Edge*));

    // —тек отката Ч вершины текущего пути. pos = длина пути. 
    path[0] = 1;
    used[1] = 1;
    int pos = 1;
    iter[1] = g->adj[1];

    int found = 0;

    while (pos > 0) {
        // ≈сли прошли все вершины Ч проверим замыкание 
        if (pos == N) {
            if (is_adj(g, path[pos - 1], path[0])) { found = 1; break; }
            // Ќет замыкани€ Ч откат 
            pos--;
            used[path[pos]] = 0;
            continue;
        }

        int u = path[pos - 1];
        Edge* e = iter[u];

        // »щем следующего непосещЄнного соседа 
        while (e && used[e->v]) e = e->next;
        iter[u] = e;

        if (e == NULL) {
            // ” вершины u больше нет вариантов Ч откат 
            used[u] = 0;
            iter[u] = NULL;
            pos--;
            continue;
        }

        // ЅерЄм соседа e->v, идЄм вглубь 
        int v = e->v;
        iter[u] = e->next;   // следующий раз начнЄм со следующего узла 
        path[pos] = v;
        used[v] = 1;
        iter[v] = g->adj[v];
        pos++;
    }

    free(iter);
    free(used);
    return found;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <graph-file>\n", argv[0]);
        return 1;
    }

    Graph g;
    if (!read_graph(argv[1], &g)) return 1;

    int* visited = (int*)malloc((g.n + 1) * sizeof(int));
    int* buf = (int*)malloc((g.n + 1) * sizeof(int));
    if (!visited || !buf) {
        free(visited); free(buf); free_graph(&g); return 1;
    }

    printf("Fayl: %s\n", argv[1]);
    printf("Vershin: %d, ryober: %d\n\n", g.n, g.m);

    // Ё…Ћ≈– 
    printf("=== EYLEROV CIKL ===\n");
    if (has_euler_circuit(&g, visited, buf)) {
        printf("Kriteriy vypolnen: graf svyaznyy, vse stepeni chetnye.\n");

        int* path = (int*)malloc((g.m + 2) * sizeof(int));
        int len = euler_circuit(&g, path);
        printf("Eylerov cikl (%d vershin): ", len);
        for (int i = 0; i < len; i++) printf("%d ", path[i]);
        printf("\n");
        free(path);
    }
    else {
        printf("Eylerova cikla net.\n");
        if (!is_connected(&g, visited, buf))
            printf("  Prichina: graf nesvyaznyy.\n");
        else {
            int odd = 0;
            for (int i = 1; i <= g.n; i++)
                if (degree(&g, i) % 2 != 0) odd++;
            if (odd > 0)
                printf("  Prichina: %d vershin s nechetnoy stepenyu.\n", odd);
        }
    }
    printf("\n");

    // √јћ»Ћ№“ќЌ
    printf("=== GAMILTONOV CIKL ===\n");
    int* hpath = (int*)malloc((g.n + 1) * sizeof(int));
    if (hamiltonian_circuit(&g, hpath)) {
        printf("Gamiltonov cikl (%d vershin): ", g.n);
        for (int i = 0; i < g.n; i++) printf("%d ", hpath[i]);
        printf("%d\n", hpath[0]);
    }
    else {
        printf("Gamiltonova cikla net.\n");
    }
    free(hpath);

    free(visited);
    free(buf);
    free_graph(&g);
    return 0;
}