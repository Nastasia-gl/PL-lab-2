#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define WORD_LEN 16          
#define MAX_TERMS 8         

char symbols[26][3] = { 0 };   

typedef struct {
    int count;                       
    char words[MAX_TERMS][WORD_LEN]; // words[0] Ч результат
    int  val[26];                    // цифра дл€ буквы 
    bool used[10];                   // какие цифры уже зан€ты 
    bool leading[26];                // маркер дл€ ведущих нулей
    long long numbers[MAX_TERMS]; 
    int word_lens[MAX_TERMS];
} Words;


void parse_string(const char* input, Words* w);
void count_letters(Words* w);
long long word_value(Words* w, const char* word);
bool check_sum(Words* w);
bool check_partial_sum(Words* w);
bool solve_rec(Words* w, int idx);
bool solve(Words* w);
void make_number(Words* w);

void parse_string(const char* input, Words* w) {
    w->count = 0;
    int ind_result = -1;

    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '+') w->count++;
        if (input[i] == '=') ind_result = i;
    }

    int n_terms = w->count + 1;

    int term = 1;           
    int len = 0;
    for (int i = 0; i < ind_result; i++) {
        if (input[i] == '+') {
            w->words[term][len] = '\0';
            term++;
            len = 0;
        }
        else if (input[i] != ' ') {
            if (len < WORD_LEN - 1) w->words[term][len++] = input[i];
        }
    }
    if (term <= n_terms) {
        w->words[term][len] = '\0';
    }

    len = 0;
    for (int i = ind_result + 1; input[i] != '\0'; i++) {
        if (input[i] != ' ') {
            if (len < WORD_LEN - 1) w->words[0][len++] = input[i];
        }
    }
    w->words[0][len] = '\0';

    w->count = n_terms; 
}

void count_letters(Words* w) {
    for (int i = 0; i < 26; i++) {
        symbols[i][0] = 'A' + i;
        symbols[i][1] = 0;
        symbols[i][2] = 0;
        w->val[i] = -1;
        w->leading[i] = false;
    }
    for (int d = 0; d < 10; d++) w->used[d] = false;

    for (int i = 0; i <= w->count; i++) {
        w->word_lens[i] = (int)strlen(w->words[i]);
        for (int j = 0; w->words[i][j] != '\0'; j++) {
            int symb = w->words[i][j] - 'A';
            if (symb < 0 || symb > 25) continue;
            symbols[symb][1] = 1;
        }
        if (w->words[i][0] != '\0') {
            w->leading[w->words[i][0] - 'A'] = true;
        }
    }
}

long long word_value(Words* w, const char* word) {
    long long v = 0;
    for (int i = 0; word[i] != '\0'; i++) {
        int d = w->val[word[i] - 'A'];
        if (d < 0) return -1;
        v = v * 10 + d;
    }
    return v;
}

bool check_sum(Words* w) {
    long long sum = 0;
    for (int i = 1; i <= w->count; i++) {
        long long v = word_value(w, w->words[i]);
        if (v < 0) return false;
        sum += v;
    }
    long long res = word_value(w, w->words[0]);
    return res >= 0 && sum == res;
}

bool check_partial_sum(Words* w) {
    int max_len = 0;
    for (int i = 0; i <= w->count; i++) {
        int len = w->word_lens[i];
        if (len > max_len) max_len = len;
    }
    int carry = 0;
    for (int pos = 0; pos < max_len; pos++) {
        int sum = carry;
        bool col_complete = true; //у всех ли есть цифра в разр€де
        for (int i = 1; i <= w->count; i++) {
            int len = w->word_lens[i];
            if (pos < len) {
                int l = w->words[i][len - 1 - pos] - 'A';
                if (w->val[l] < 0) { col_complete = false; break; }
                sum += w->val[l];
            }
        }
        if (!col_complete) return true;
        int res_len = w->word_lens[0];
        if (pos < res_len) {
            int l = w->words[0][res_len - 1 - pos] - 'A';
            int res_digit = w->val[l];
            if (res_digit < 0) return true;
            if ((sum % 10) != res_digit) return false;
        }
        else {
            if ((sum % 10) != 0) return false;
        }
        carry = sum / 10;
    }
    if (carry != 0) return false;
    return true;
}

bool solve_rec(Words* w, int idx) {
    static int letters[26];
    static int nletters = -1;
    if (nletters < 0) {
        nletters = 0;
        for (int i = 0; i < 26; i++)
            if (symbols[i][1] == 1) letters[nletters++] = i;
    }
    if (idx == nletters) return check_sum(w);
    int l = letters[idx];
    for (int d = 0; d <= 9; d++) {
        if (w->used[d]) continue;
        if (d == 0 && w->leading[l]) continue;
        w->val[l] = d;
        w->used[d] = true;
        if (check_partial_sum(w)) {
            if (solve_rec(w, idx + 1)) return true;
        }
        w->val[l] = -1;
        w->used[d] = false;
    }
    return false;
}

bool solve(Words* w) {
    return solve_rec(w, 0);
}

void make_number(Words* w) {
    for (int i = 0; i <= w->count; i++) {
        w->numbers[i] = word_value(w, w->words[i]);
    }
}

int main(void) {
    char input[256];
    Words w;
    memset(&w, 0, sizeof(w));

    printf("Enter the rebus: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Error\n");
        return 1;
    }
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') input[len - 1] = '\0';

    parse_string(input, &w);

    count_letters(&w);
    
    clock_t start_time = clock();

    bool found = solve(&w);

    clock_t end_time = clock();

    if (!found) {
        printf("No solution\n");
        return 1;
    }

    make_number(&w);
  

    printf("Solution: ");
    for (int i = 1; i < w.count; i++) printf("%lld + ", w.numbers[i]);
    printf("%lld = %lld\n", w.numbers[w.count], w.numbers[0]);

    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Time: %.4f seconds\n", time_spent);

    return 0;
}