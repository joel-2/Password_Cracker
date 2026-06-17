#include <string.h>   // for strcpy, strlen
#include <ctype.h>    // for toupper
#include <stdio.h>    // for sprintf
#include "rules.h"   // for RuleFn and rule function declarations

// ─── RULE FUNCTIONS ───────────────────────────────────────────────

// capitalises the first letter only: password → Password
void rule_capitalise(const char *word, char *out) {
    strcpy(out, word);                          // copy word into out unchanged
    if (out[0] >= 'a' && out[0] <= 'z') {      // check if first char is lowercase
        out[0] -= 32;                           // ASCII trick: 'a' is 97, 'A' is 65, difference is 32
    }
}

// converts entire word to uppercase: password → PASSWORD
void rule_uppercase(const char *word, char *out) {
    int i = 0;
    while (word[i]) {                           // loop until null terminator
        out[i] = toupper(word[i]);              // toupper handles non-letters safely (leaves them unchanged)
        i++;
    }
    out[i] = '\0';                              // null terminate
}

// basic leet speak substitutions: password → p@55w0rd
void rule_leet(const char *word, char *out) {
    int i = 0;
    while (word[i]) {
        switch (word[i]) {
            case 'a': case 'A': out[i] = '@'; break;   // a → @
            case 'e': case 'E': out[i] = '3'; break;   // e → 3
            case 'i': case 'I': out[i] = '1'; break;   // i → 1
            case 'o': case 'O': out[i] = '0'; break;   // o → 0
            case 's': case 'S': out[i] = '$'; break;   // s → $
            case 't': case 'T': out[i] = '7'; break;   // t → 7
            default:            out[i] = word[i]; break; // leave everything else unchanged
        }
        i++;
    }
    out[i] = '\0';                              // null terminate
}

// appends 123: password → password123
void rule_append_123(const char *word, char *out) {
    sprintf(out, "%s123", word);                // sprintf builds the new string directly
}

// appends !: password → password!
void rule_append_exclamation(const char *word, char *out) {
    sprintf(out, "%s!", word);
}

// appends current year: password → password2024
void rule_append_year(const char *word, char *out) {
    sprintf(out, "%s2024", word);
}

// reverses the word: password → drowssap
void rule_reverse(const char *word, char *out) {
    int len = strlen(word);
    for (int i = 0; i < len; i++) {
        out[i] = word[len - 1 - i];            // copy from end to start
    }
    out[len] = '\0';                            // null terminate
}

// capitalise + leet + append 123: password → P@55w0rd123
void rule_combined(const char *word, char *out) {
    char temp1[256];
    char temp2[256];
    rule_capitalise(word, temp1);               // step 1: capitalise
    rule_leet(temp1, temp2);                    // step 2: leet the capitalised version
    sprintf(out, "%s123", temp2);               // step 3: append 123
}

void rule_cap_append_123(const char *word, char *out) {
    char temp[256];
    rule_capitalise(word, temp);    // step 1: capitalise
    sprintf(out, "%s123", temp);    // step 2: append 123
}

void rule_caps_numbers_append(const char *word, char *out) {
    char temp[256];
    rule_uppercase(word, temp);     // step 1: uppercase
    sprintf(out, "%s123", temp);    // step 2: append 123
}

// ─── RULE REGISTRY ────────────────────────────────────────────────
RuleFn rules[] = {
    rule_capitalise,
    rule_uppercase,
    rule_leet,
    rule_append_123,
    rule_append_exclamation,
    rule_append_year,
    rule_reverse,
    rule_combined,
    rule_cap_append_123,
    rule_caps_numbers_append,
};

int rule_count = sizeof(rules) / sizeof(rules[0]);  // automatically counts how many rules there are
