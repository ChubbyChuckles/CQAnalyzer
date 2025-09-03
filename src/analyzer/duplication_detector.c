#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "analyzer/duplication_detector.h"
#include "utils/logger.h"

// Simple hash function for strings
static unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Structure for token sequence tracking
typedef struct {
    char **tokens;
    int count;
    int capacity;
} TokenList;

static void init_token_list(TokenList *list) {
    list->tokens = NULL;
    list->count = 0;
    list->capacity = 0;
}

static void add_token(TokenList *list, const char *token) {
    if (list->count >= list->capacity) {
        list->capacity = list->capacity == 0 ? 16 : list->capacity * 2;
        list->tokens = realloc(list->tokens, list->capacity * sizeof(char *));
    }
    size_t len = strlen(token) + 1;
    list->tokens[list->count] = malloc(len);
    if (list->tokens[list->count]) {
        strcpy(list->tokens[list->count], token);
        list->count++;
    }
}

static void free_token_list(TokenList *list) {
    for (int i = 0; i < list->count; i++) {
        free(list->tokens[i]);
    }
    free(list->tokens);
}

// Tokenize a line of code
static void tokenize_line(const char *line, TokenList *tokens) {
    const char *ptr = line;
    char token[256];
    int token_len = 0;

    while (*ptr) {
        if (isalnum(*ptr) || *ptr == '_') {
            // Identifier or number
            token[token_len++] = *ptr;
        } else if (isspace(*ptr)) {
            // End of token
            if (token_len > 0) {
                token[token_len] = '\0';
                add_token(tokens, token);
                token_len = 0;
            }
        } else {
            // Operator or punctuation
            if (token_len > 0) {
                token[token_len] = '\0';
                add_token(tokens, token);
                token_len = 0;
            }
            // Add single character as token
            token[0] = *ptr;
            token[1] = '\0';
            add_token(tokens, token);
        }
        ptr++;
    }

    // Add final token
    if (token_len > 0) {
        token[token_len] = '\0';
        add_token(tokens, token);
    }
}

CQError detect_file_duplication(const char *filepath, double *duplication_ratio)
{
    if (!filepath || !duplication_ratio)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        LOG_ERROR("Could not open file for duplication detection: %s", filepath);
        return CQ_ERROR_FILE_NOT_FOUND;
    }

    TokenList tokens;
    init_token_list(&tokens);

    char line[1024];
    while (fgets(line, sizeof(line), file))
    {
        // Skip comments and empty lines for simplicity
        if (line[0] == '\0' || strstr(line, "//") || strstr(line, "/*"))
            continue;

        tokenize_line(line, &tokens);
    }
    fclose(file);

    if (tokens.count < 10)
    {
        // Too few tokens for meaningful duplication detection
        free_token_list(&tokens);
        *duplication_ratio = 0.0;
        return CQ_SUCCESS;
    }

    // Simple duplication detection using sliding window
    const int window_size = 5; // Look for sequences of 5 tokens
    int duplicated_tokens = 0;
    int total_sequences = tokens.count - window_size + 1;

    // Use a simple hash-based approach
    #define HASH_SIZE 1024
    unsigned long hashes[HASH_SIZE] = {0};
    int counts[HASH_SIZE] = {0};

    for (int i = 0; i < total_sequences; i++)
    {
        // Create hash of token sequence
        unsigned long seq_hash = 0;
        for (int j = 0; j < window_size; j++)
        {
            seq_hash = seq_hash * 31 + hash_string(tokens.tokens[i + j]);
        }

        int hash_index = seq_hash % HASH_SIZE;
        if (counts[hash_index] > 0 && hashes[hash_index] == seq_hash)
        {
            duplicated_tokens += window_size;
        }
        hashes[hash_index] = seq_hash;
        counts[hash_index]++;
    }

    // Calculate duplication ratio
    int total_tokens = tokens.count;
    if (total_tokens > 0)
    {
        *duplication_ratio = (double)duplicated_tokens / (double)total_tokens;
        if (*duplication_ratio > 1.0) *duplication_ratio = 1.0;
    }
    else
    {
        *duplication_ratio = 0.0;
    }

    free_token_list(&tokens);

    LOG_INFO("Duplication detection for %s: ratio=%.3f", filepath, *duplication_ratio);
    return CQ_SUCCESS;
}

CQError detect_project_duplication(const char **filepaths, int num_files, double *duplication_ratio)
{
    if (!filepaths || num_files <= 0 || !duplication_ratio)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // TODO: Implement project-wide duplication detection
    // TODO: Compare all file pairs
    // TODO: Aggregate duplication metrics

    LOG_WARNING("Project duplication detection not yet implemented");
    *duplication_ratio = 0.0; // No duplication detected

    return CQ_SUCCESS;
}
