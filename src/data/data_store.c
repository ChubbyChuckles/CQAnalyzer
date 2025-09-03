#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cqanalyzer.h"
#include "data/data_store.h"
#include "utils/logger.h"

// Simple hash table for storing file data
#define HASH_TABLE_SIZE 1024

typedef struct MetricEntry
{
    char metric_name[64];
    double value;
    struct MetricEntry *next;
} MetricEntry;

typedef struct FileEntry
{
    char filepath[MAX_PATH_LENGTH];
    SupportedLanguage language;
    MetricEntry *metrics;
    struct FileEntry *next;
} FileEntry;

static FileEntry *file_hash_table[HASH_TABLE_SIZE];
static bool data_store_initialized = false;

static unsigned int hash_string(const char *str)
{
    unsigned int hash = 0;
    while (*str)
    {
        hash = (hash * 31) + *str++;
    }
    return hash % HASH_TABLE_SIZE;
}

CQError data_store_init(void)
{
    if (data_store_initialized)
    {
        return CQ_SUCCESS;
    }

    memset(file_hash_table, 0, sizeof(file_hash_table));
    data_store_initialized = true;

    LOG_INFO("Data store initialized");
    return CQ_SUCCESS;
}

void data_store_shutdown(void)
{
    if (!data_store_initialized)
    {
        return;
    }

    // Free all allocated memory
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        FileEntry *entry = file_hash_table[i];
        while (entry)
        {
            FileEntry *next_entry = entry->next;

            MetricEntry *metric = entry->metrics;
            while (metric)
            {
                MetricEntry *next_metric = metric->next;
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

CQError data_store_add_file(const char *filepath, SupportedLanguage language)
{
    if (!data_store_initialized || !filepath)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    unsigned int hash = hash_string(filepath);

    // Check if file already exists
    FileEntry *entry = file_hash_table[hash];
    while (entry)
    {
        if (strcmp(entry->filepath, filepath) == 0)
        {
            // Update language if different
            entry->language = language;
            return CQ_SUCCESS;
        }
        entry = entry->next;
    }

    // Create new entry
    entry = (FileEntry *)malloc(sizeof(FileEntry));
    if (!entry)
    {
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

CQError data_store_add_metric(const char *filepath, const char *metric_name, double value)
{
    if (!data_store_initialized || !filepath || !metric_name)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    unsigned int hash = hash_string(filepath);

    // Find file entry
    FileEntry *entry = file_hash_table[hash];
    while (entry)
    {
        if (strcmp(entry->filepath, filepath) == 0)
        {
            break;
        }
        entry = entry->next;
    }

    if (!entry)
    {
        LOG_ERROR("File not found in data store: %s", filepath);
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Check if metric already exists
    MetricEntry *metric = entry->metrics;
    while (metric)
    {
        if (strcmp(metric->metric_name, metric_name) == 0)
        {
            // Update value
            metric->value = value;
            return CQ_SUCCESS;
        }
        metric = metric->next;
    }

    // Create new metric entry
    metric = (MetricEntry *)malloc(sizeof(MetricEntry));
    if (!metric)
    {
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

int data_store_get_all_files(char filepaths[][MAX_PATH_LENGTH], int max_files)
{
    if (!data_store_initialized || !filepaths || max_files <= 0)
    {
        return 0;
    }

    int count = 0;

    for (int i = 0; i < HASH_TABLE_SIZE && count < max_files; i++)
    {
        FileEntry *entry = file_hash_table[i];
        while (entry && count < max_files)
        {
            strcpy(filepaths[count], entry->filepath);
            count++;
            entry = entry->next;
        }
    }

    return count;
}

int data_store_get_all_metric_values(const char *metric_name, double *values, int max_values)
{
    if (!data_store_initialized || !metric_name || !values || max_values <= 0)
    {
        return 0;
    }

    int count = 0;

    for (int i = 0; i < HASH_TABLE_SIZE && count < max_values; i++)
    {
        FileEntry *entry = file_hash_table[i];
        while (entry && count < max_values)
        {
            // Find the metric for this file
            MetricEntry *metric = entry->metrics;
            while (metric && count < max_values)
            {
                if (strcmp(metric->metric_name, metric_name) == 0)
                {
                    values[count] = metric->value;
                    count++;
                    break; // Only one value per file per metric
                }
                metric = metric->next;
            }
            entry = entry->next;
        }
    }

    return count;
}

double data_store_get_metric(const char *filepath, const char *metric_name)
{
    if (!data_store_initialized || !filepath || !metric_name)
    {
        return -1.0;
    }
    
    // Binary serialization format:
    // - Header: "CQDS" magic + version (uint32_t) + num_files (uint32_t)
    // - For each file: filepath_len (uint32_t) + filepath + language (uint32_t) + num_metrics (uint32_t)
    // - For each metric: metric_name_len (uint32_t) + metric_name + value (double)
    
    // Simple compression using run-length encoding for repeated values
    static size_t compress_data(const unsigned char *input, size_t input_size,
                               unsigned char **output)
    {
        if (!input || input_size == 0 || !output)
        {
            return 0;
        }
    
        // For now, implement a simple copy (no compression) that can be enhanced later
        // This provides the framework for adding compression algorithms like zlib
        *output = (unsigned char *)malloc(input_size);
        if (!*output)
        {
            return 0;
        }
    
        memcpy(*output, input, input_size);
        return input_size;
    }
    
    static size_t decompress_data(const unsigned char *input, size_t input_size,
                                 unsigned char **output, size_t expected_size)
    {
        if (!input || input_size == 0 || !output || expected_size == 0)
        {
            return 0;
        }
    
        // For now, implement a simple copy (no decompression)
        *output = (unsigned char *)malloc(expected_size);
        if (!*output)
        {
            return 0;
        }
    
        memcpy(*output, input, input_size);
        return input_size;
    }
    
    CQError data_store_serialize_binary(const char *filepath)
    {
        if (!data_store_initialized || !filepath)
        {
            return CQ_ERROR_INVALID_ARGUMENT;
        }
    
        FILE *file = fopen(filepath, "wb");
        if (!file)
        {
            LOG_ERROR("Failed to open file for binary serialization: %s", filepath);
            return CQ_ERROR_FILE_NOT_FOUND;
        }
    
        // Write header with compression flag
        const char *magic = "CQDS";
        uint32_t version = 1;
        uint32_t flags = 0; // Bit 0: compression enabled
        uint32_t num_files = 0;
    
        // Count total files
        for (int i = 0; i < HASH_TABLE_SIZE; i++)
        {
            FileEntry *entry = file_hash_table[i];
            while (entry)
            {
                num_files++;
                entry = entry->next;
            }
        }
    
        fwrite(magic, sizeof(char), 4, file);
        fwrite(&version, sizeof(uint32_t), 1, file);
        fwrite(&flags, sizeof(uint32_t), 1, file);
        fwrite(&num_files, sizeof(uint32_t), 1, file);
    
        // Write file data
        for (int i = 0; i < HASH_TABLE_SIZE; i++)
        {
            FileEntry *entry = file_hash_table[i];
            while (entry)
            {
                // Write filepath
                uint32_t filepath_len = strlen(entry->filepath) + 1; // +1 for null terminator
                fwrite(&filepath_len, sizeof(uint32_t), 1, file);
                fwrite(entry->filepath, sizeof(char), filepath_len, file);
    
                // Write language
                uint32_t language = (uint32_t)entry->language;
                fwrite(&language, sizeof(uint32_t), 1, file);
    
                // Count and write metrics
                uint32_t num_metrics = 0;
                MetricEntry *metric = entry->metrics;
                while (metric)
                {
                    num_metrics++;
                    metric = metric->next;
                }
                fwrite(&num_metrics, sizeof(uint32_t), 1, file);
    
                // Write metrics
                metric = entry->metrics;
                while (metric)
                {
                    uint32_t name_len = strlen(metric->metric_name) + 1;
                    fwrite(&name_len, sizeof(uint32_t), 1, file);
                    fwrite(metric->metric_name, sizeof(char), name_len, file);
                    fwrite(&metric->value, sizeof(double), 1, file);
                    metric = metric->next;
                }
    
                entry = entry->next;
            }
        }
    
        fclose(file);
        LOG_INFO("Data store serialized to binary file: %s", filepath);
        return CQ_SUCCESS;
    }
    
    CQError data_store_deserialize_binary(const char *filepath)
    {
        if (!data_store_initialized || !filepath)
        {
            return CQ_ERROR_INVALID_ARGUMENT;
        }
    
        FILE *file = fopen(filepath, "rb");
        if (!file)
        {
            LOG_ERROR("Failed to open file for binary deserialization: %s", filepath);
            return CQ_ERROR_FILE_NOT_FOUND;
        }
    
        // Read and verify header
        char magic[5] = {0};
        uint32_t version, flags, num_files;
    
        if (fread(magic, sizeof(char), 4, file) != 4 ||
            strncmp(magic, "CQDS", 4) != 0)
        {
            LOG_ERROR("Invalid binary file format");
            fclose(file);
            return CQ_ERROR_UNKNOWN;
        }
    
        if (fread(&version, sizeof(uint32_t), 1, file) != 1 ||
            fread(&flags, sizeof(uint32_t), 1, file) != 1 ||
            fread(&num_files, sizeof(uint32_t), 1, file) != 1)
        {
            LOG_ERROR("Failed to read binary file header");
            fclose(file);
            return CQ_ERROR_UNKNOWN;
        }
    
        if (version != 1)
        {
            LOG_ERROR("Unsupported binary file version: %u", version);
            fclose(file);
            return CQ_ERROR_UNKNOWN;
        }
    
        // Validate reasonable limits
        if (num_files > 100000) // Reasonable upper limit
        {
            LOG_ERROR("Too many files in cache: %u", num_files);
            fclose(file);
            return CQ_ERROR_UNKNOWN;
        }
    
        // Clear existing data
        data_store_shutdown();
        data_store_init();
    
        // Read file data
        for (uint32_t i = 0; i < num_files; i++)
        {
            // Read filepath
            uint32_t filepath_len;
            if (fread(&filepath_len, sizeof(uint32_t), 1, file) != 1)
            {
                LOG_ERROR("Failed to read filepath length");
                fclose(file);
                return CQ_ERROR_UNKNOWN;
            }
    
            // Validate filepath length
            if (filepath_len == 0 || filepath_len > MAX_PATH_LENGTH)
            {
                LOG_ERROR("Invalid filepath length: %u", filepath_len);
                fclose(file);
                return CQ_ERROR_UNKNOWN;
            }
    
            char *filepath_buf = (char *)malloc(filepath_len);
            if (!filepath_buf)
            {
                LOG_ERROR("Failed to allocate memory for filepath");
                fclose(file);
                return CQ_ERROR_MEMORY_ALLOCATION;
            }
    
            if (fread(filepath_buf, sizeof(char), filepath_len, file) != filepath_len)
            {
                LOG_ERROR("Failed to read filepath");
                free(filepath_buf);
                fclose(file);
                return CQ_ERROR_UNKNOWN;
            }
    
            // Validate filepath is null-terminated
            if (filepath_buf[filepath_len - 1] != '\0')
            {
                LOG_ERROR("Filepath not null-terminated");
                free(filepath_buf);
                fclose(file);
                return CQ_ERROR_UNKNOWN;
            }
    
            // Read language
            uint32_t language_val;
            if (fread(&language_val, sizeof(uint32_t), 1, file) != 1)
            {
                LOG_ERROR("Failed to read language");
                free(filepath_buf);
                fclose(file);
                return CQ_ERROR_UNKNOWN;
            }
    
            // Validate language enum
            if (language_val >= LANG_UNKNOWN)
            {
                LOG_ERROR("Invalid language value: %u", language_val);
                free(filepath_buf);
                fclose(file);
                return CQ_ERROR_UNKNOWN;
            }
    
            SupportedLanguage language = (SupportedLanguage)language_val;
    
            // Add file
            CQError result = data_store_add_file(filepath_buf, language);
            if (result != CQ_SUCCESS)
            {
                LOG_ERROR("Failed to add file to data store");
                free(filepath_buf);
                fclose(file);
                return result;
            }
    
            // Read metrics
            uint32_t num_metrics;
            if (fread(&num_metrics, sizeof(uint32_t), 1, file) != 1)
            {
                LOG_ERROR("Failed to read number of metrics");
                free(filepath_buf);
                fclose(file);
                return CQ_ERROR_UNKNOWN;
            }
    
            // Validate reasonable number of metrics per file
            if (num_metrics > 1000)
            {
                LOG_ERROR("Too many metrics for file %s: %u", filepath_buf, num_metrics);
                free(filepath_buf);
                fclose(file);
                return CQ_ERROR_UNKNOWN;
            }
    
            for (uint32_t j = 0; j < num_metrics; j++)
            {
                uint32_t name_len;
                if (fread(&name_len, sizeof(uint32_t), 1, file) != 1)
                {
                    LOG_ERROR("Failed to read metric name length");
                    free(filepath_buf);
                    fclose(file);
                    return CQ_ERROR_UNKNOWN;
                }
    
                // Validate metric name length
                if (name_len == 0 || name_len > 64)
                {
                    LOG_ERROR("Invalid metric name length: %u", name_len);
                    free(filepath_buf);
                    fclose(file);
                    return CQ_ERROR_UNKNOWN;
                }
    
                char *metric_name = (char *)malloc(name_len);
                if (!metric_name)
                {
                    LOG_ERROR("Failed to allocate memory for metric name");
                    free(filepath_buf);
                    fclose(file);
                    return CQ_ERROR_MEMORY_ALLOCATION;
                }
    
                if (fread(metric_name, sizeof(char), name_len, file) != name_len)
                {
                    LOG_ERROR("Failed to read metric name");
                    free(metric_name);
                    free(filepath_buf);
                    fclose(file);
                    return CQ_ERROR_UNKNOWN;
                }
    
                // Validate metric name is null-terminated
                if (metric_name[name_len - 1] != '\0')
                {
                    LOG_ERROR("Metric name not null-terminated");
                    free(metric_name);
                    free(filepath_buf);
                    fclose(file);
                    return CQ_ERROR_UNKNOWN;
                }
    
                double value;
                if (fread(&value, sizeof(double), 1, file) != 1)
                {
                    LOG_ERROR("Failed to read metric value");
                    free(metric_name);
                    free(filepath_buf);
                    fclose(file);
                    return CQ_ERROR_UNKNOWN;
                }
    
                // Validate metric value is reasonable (not NaN or infinite)
                if (isnan(value) || isinf(value))
                {
                    LOG_ERROR("Invalid metric value for %s: %f", metric_name, value);
                    free(metric_name);
                    free(filepath_buf);
                    fclose(file);
                    return CQ_ERROR_UNKNOWN;
                }
    
                result = data_store_add_metric(filepath_buf, metric_name, value);
                if (result != CQ_SUCCESS)
                {
                    LOG_ERROR("Failed to add metric to data store");
                    free(metric_name);
                    free(filepath_buf);
                    fclose(file);
                    return result;
                }
    
                free(metric_name);
            }
    
            free(filepath_buf);
        }
    
        fclose(file);
        LOG_INFO("Data store deserialized from binary file: %s", filepath);
        return CQ_SUCCESS;
    }
    
    CQError data_store_serialize_json(const char *filepath)
    {
        if (!data_store_initialized || !filepath)
        {
            return CQ_ERROR_INVALID_ARGUMENT;
        }
    
        FILE *file = fopen(filepath, "w");
        if (!file)
        {
            LOG_ERROR("Failed to open file for JSON serialization: %s", filepath);
            return CQ_ERROR_FILE_NOT_FOUND;
        }
    
        fprintf(file, "{\n");
        fprintf(file, "  \"version\": \"1.0\",\n");
        fprintf(file, "  \"files\": [\n");
    
        bool first_file = true;
        for (int i = 0; i < HASH_TABLE_SIZE; i++)
        {
            FileEntry *entry = file_hash_table[i];
            while (entry)
            {
                if (!first_file)
                {
                    fprintf(file, ",\n");
                }
                first_file = false;
    
                fprintf(file, "    {\n");
                fprintf(file, "      \"filepath\": \"%s\",\n", entry->filepath);
                fprintf(file, "      \"language\": \"%s\",\n", language_to_string(entry->language));
                fprintf(file, "      \"metrics\": {\n");
    
                bool first_metric = true;
                MetricEntry *metric = entry->metrics;
                while (metric)
                {
                    if (!first_metric)
                    {
                        fprintf(file, ",\n");
                    }
                    first_metric = false;
    
                    fprintf(file, "        \"%s\": %.6f", metric->metric_name, metric->value);
                    metric = metric->next;
                }
                fprintf(file, "\n      }\n");
                fprintf(file, "    }");
    
                entry = entry->next;
            }
        }
    
        fprintf(file, "\n  ]\n");
        fprintf(file, "}\n");
    
        fclose(file);
        LOG_INFO("Data store serialized to JSON file: %s", filepath);
        return CQ_SUCCESS;
    }
    
    CQError data_store_export_csv(const char *filepath)
    {
        if (!data_store_initialized || !filepath)
        {
            return CQ_ERROR_INVALID_ARGUMENT;
        }
    
        FILE *file = fopen(filepath, "w");
        if (!file)
        {
            LOG_ERROR("Failed to open file for CSV export: %s", filepath);
            return CQ_ERROR_FILE_NOT_FOUND;
        }
    
        // Write CSV header
        fprintf(file, "Filepath,Language");
    
        // Collect all unique metric names
        const int MAX_METRICS = 100;
        char metric_names[MAX_METRICS][64];
        int num_unique_metrics = 0;
    
        for (int i = 0; i < HASH_TABLE_SIZE && num_unique_metrics < MAX_METRICS; i++)
        {
            FileEntry *entry = file_hash_table[i];
            while (entry && num_unique_metrics < MAX_METRICS)
            {
                MetricEntry *metric = entry->metrics;
                while (metric && num_unique_metrics < MAX_METRICS)
                {
                    bool found = false;
                    for (int j = 0; j < num_unique_metrics; j++)
                    {
                        if (strcmp(metric_names[j], metric->metric_name) == 0)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        strcpy(metric_names[num_unique_metrics], metric->metric_name);
                        num_unique_metrics++;
                    }
                    metric = metric->next;
                }
                entry = entry->next;
            }
        }
    
        // Write metric headers
        for (int i = 0; i < num_unique_metrics; i++)
        {
            fprintf(file, ",%s", metric_names[i]);
        }
        fprintf(file, "\n");
    
        // Write data rows
        for (int i = 0; i < HASH_TABLE_SIZE; i++)
        {
            FileEntry *entry = file_hash_table[i];
            while (entry)
            {
                fprintf(file, "\"%s\",%s", entry->filepath, language_to_string(entry->language));
    
                // Write metric values
                for (int j = 0; j < num_unique_metrics; j++)
                {
                    double value = data_store_get_metric(entry->filepath, metric_names[j]);
                    if (value >= 0.0)
                    {
                        fprintf(file, ",%.6f", value);
                    }
                    else
                    {
                        fprintf(file, ",");
                    }
                }
                fprintf(file, "\n");
    
                entry = entry->next;
            }
        }
    
        fclose(file);
        LOG_INFO("Data store exported to CSV file: %s", filepath);
        return CQ_SUCCESS;
    }
    

    unsigned int hash = hash_string(filepath);

    // Find file entry
    FileEntry *entry = file_hash_table[hash];
    while (entry)
    {
        if (strcmp(entry->filepath, filepath) == 0)
        {
            break;
        }
        entry = entry->next;
    }

    if (!entry)
    {
        return -1.0;
    }

    // Find metric
    MetricEntry *metric = entry->metrics;
    while (metric)
    {
        if (strcmp(metric->metric_name, metric_name) == 0)
        {
            return metric->value;
        }
        metric = metric->next;
    }

    return -1.0;
}
