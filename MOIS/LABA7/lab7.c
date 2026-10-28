#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

// НОД двух чисел (для вычисления НОК) 
static int gcd(int a, int b) {
    while (b) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

// НОК двух чисел 
static int lcm(int a, int b) {
    return a / gcd(a, b) * b;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Oshibka (nuzhno imya fayla)\n");
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        printf("Oshibka (fayl ne otkryt)\n");
        return 1;
    }

    int n = 0;
    if (fscanf(f, "%d", &n) != 1 || n <= 0) {
        printf("Oshibka (nekorrektnoe n)\n");
        fclose(f);
        return 1;
    }

    int* p = (int*)malloc((n + 1) * sizeof(int));
    int* used = (int*)calloc(n + 1, sizeof(int));
    if (!p || !used) {
        printf("Oshibka (net pamyati)\n");
        free(p); free(used); fclose(f);
        return 1;
    }

    for (int i = 1; i <= n; i++) {
        if (fscanf(f, "%d", &p[i]) != 1 || p[i] < 1 || p[i] > n) {
            printf("Oshibka (nekorrektnaya perestanovka)\n");
            free(p); free(used); fclose(f);
            return 1;
        }
    }
    fclose(f);

    // 1. РАЗЛОЖЕНИЕ НА ЦИКЛЫ И ПОИСК СТЕПЕНИ
    printf("Razlozhenie na nezavisimye cikly:\n");
    int degree = 1;
    int transpositions = 0;
    int num_cycles = 0;

    for (int i = 1; i <= n; i++) {
        if (!used[i]) {
            num_cycles++;
            int cycle_len = 0;
            int curr = i;
            printf("(");
            while (!used[curr]) {
                used[curr] = 1;
                printf("%d ", curr);
                curr = p[curr];
                cycle_len++;
            }
            printf(") ");

            degree = lcm(degree, cycle_len);
            transpositions += (cycle_len - 1); // Цикл длины k = (k-1) транспозиция
        }
    }
    printf("\n\n");

    // 2. СТЕПЕНЬ
    printf("Stepen podstanovki: %d\n\n", degree);

    // 3. ЧЕТНОСТЬ
    // Способ А: через транспозиции
    printf("Chetnost (cherez transpozicii):\n");
    printf("  Chislo transpoziciy: %d\n", transpositions);
    int parity_transp = transpositions % 2; // 0 - четная, 1 - нечетная
    printf("  Podstanovka: %s\n\n", parity_transp == 0 ? "CHETNAYA" : "NECHETNAYA");

    // Способ Б: через инверсии
    printf("Chetnost (cherez inversii):\n");
    int inversions = 0;
    for (int i = 1; i <= n; i++) {
        for (int j = i + 1; j <= n; j++) {
            if (p[i] > p[j]) {
                inversions++;
            }
        }
    }
    printf("  Chislo inversiy: %d\n", inversions);
    int parity_inv = inversions % 2;
    printf("  Podstanovka: %s\n\n", parity_inv == 0 ? "CHETNAYA" : "NECHETNAYA");

    // Сверка
    if (parity_transp != parity_inv) {
        printf("VNIMANIE: rezultaty ne sovpali!\n");
    }

    free(p);
    free(used);
    return 0;
}