#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data/data_store.h"
#include "utils/logger.h"

// Simple hash table for storing file data
#define HASH_TABLE_SIZE 1024

typedef struct MetricEntry {
    char metric_name[64];
    double value;
    struct MetricEntry* next;
} MetricEntry;

typedef struct FileEntry {
    char filepath[MAX_PATH_LENGTH];
    SupportedLanguage language;
    MetricEntry* metrics;
    struct FileEntry* next;
} FileEntry;

static FileEntry* file_hash_table[HASH_TABLE_SIZE];
static bool data_store_initialized = false;

static unsigned int hash_string(const char* str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash * 31) + *str++;
    }
    return hash % HASH_TABLE_SIZE;
}

CQError data_store_init(void) {
    if (data_store_initialized) {
        return CQ_SUCCESS;
    }

    memset(file_hash_table, 0, sizeof(file_hash_table));
    data_store_initialized = true;

    LOG_INFO("Data store initialized");
    return CQ_SUCCESS;
}

void data_store_shutdown(void) {
    if (!data_store_initialized) {
        return;
    }

    // Free all allocated memory
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        FileEntry* entry = file_hash_table[i];
        while (entry) {
            FileEntry* next_entry = entry->next;

            MetricEntry* metric = entry->metrics;
            while (metric) {
                MetricEntry* next_metric = metric->next;
                free(metric);
                metric = next_metric;
            }

            free(entry);
            entry = next_entry;
        }
        file_hash_table[i] = NULL;
    }

    data_store_initialized = false;
    LOG_INFO("Data store shutdown");
}

CQError data_store_add_file(const char* filepath, SupportedLanguage language) {
    if (!data_store_initialized || !filepath) {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    unsigned int hash = hash_string(filepath);

    // Check if file already exists
    FileEntry* entry = file_hash_table[hash];
    while (entry) {
        if (strcmp(entry->filepath, filepath) == 0) {
            // Update language if different
            entry->language = language;
            return CQ_SUCCESS;
        }
        entry = entry->next;
    }

    // Create new entry
    entry = (FileEntry*)malloc(sizeof(FileEntry));
    if (!entry) {
        LOG_ERROR("Failed to allocate memory for file entry");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    strcpy(entry->filepath, filepath);
    entry->language = language;
    entry->metrics = NULL;
    entry->next = file_hash_table[hash];
    file_hash_table[hash] = entry;

    LOG_DEBUG("Added file to data store: %s", filepath);
    return CQ_SUCCESS;
}

CQError data_store_add_metric(const char* filepath, const char* metric_name, double value) {
    if (!data_store_initialized || !filepath || !metric_name) {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    unsigned int hash = hash_string(filepath);

    // Find file entry
    FileEntry* entry = file_hash_table[hash];
    while (entry) {
        if (strcmp(entry->filepath, filepath) == 0) {
            break;
        }
        entry = entry->next;
    }

    if (!entry) {
        LOG_ERROR("File not found in data store: %s", filepath);
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Check if metric already exists
    MetricEntry* metric = entry->metrics;
    while (metric) {
        if (strcmp(metric->metric_name, metric_name) == 0) {
            // Update value
            metric->value = value;
            return CQ_SUCCESS;
        }
        metric = metric->next;
    }

    // Create new metric entry
    metric = (MetricEntry*)malloc(sizeof(MetricEntry));
    if (!metric) {
        LOG_ERROR("Failed to allocate memory for metric entry");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    strcpy(metric->metric_name, metric_name);
    metric->value = value;
    metric->next = entry->metrics;
    entry->metrics = metric;

    LOG_DEBUG("Added metric %s=%.2f for file: %s", metric_name, value, filepath);
    return CQ_SUCCESS;
}

double data_store_get_metric(const char* filepath, const char* metric_name) {
    if (!data_store_initialized || !filepath || !metric_name) {
        return -1.0;
    }

    unsigned int hash = hash_string(filepath);

    // Find file entry
    FileEntry* entry = file_hash_table[hash];
    while (entry) {
        if (strcmp(entry->filepath, filepath) == 0) {
            break;
        }
        entry = entry->next;
    }

    if (!entry) {
        return -1.0;
    }

    // Find metric
    MetricEntry* metric = entry->metrics;
    while (metric) {
        if (strcmp(metric->metric_name, metric_name) == 0) {
            return metric->value;
        }
        metric = metric->next;
    }

    return -1.0;
}
