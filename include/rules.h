#ifndef RULES_H
#define RULES_H

void rule_capitalise(const char *word, char *out);
void rule_uppercase(const char *word, char *out);
void rule_leet(const char *word, char *out);
void rule_append_123(const char *word, char *out);
void rule_append_exclamation(const char *word, char *out);
void rule_append_year(const char *word, char *out);
void rule_reverse(const char *word, char *out);
void rule_combined(const char *word, char *out);

// function pointer type - every rule has this signature
typedef void (*RuleFn)(const char *word, char *out);

// array of all rules and how many there are
extern RuleFn rules[];
extern int rule_count;

#endif