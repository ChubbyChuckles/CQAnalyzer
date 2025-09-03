#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>

#include "utils/logger.h"

static LogLevel current_level = LOG_LEVEL_INFO;
static int current_outputs = LOG_OUTPUT_CONSOLE;
static FILE *log_file = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char *level_strings[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR"};

static const char *level_colors[] = {
    "\033[36m", // Cyan for DEBUG
    "\033[32m", // Green for INFO
    "\033[33m", // Yellow for WARNING
    "\033[31m"  // Red for ERROR
};

static const char *color_reset = "\033[0m";

CQError logger_init(void)
{
    // Initialize mutex
    if (pthread_mutex_init(&log_mutex, NULL) != 0)
    {
        return CQ_ERROR_UNKNOWN;
    }

    LOG_INFO("Logger initialized successfully");
    return CQ_SUCCESS;
}

void logger_shutdown(void)
{
    pthread_mutex_lock(&log_mutex);

    if (log_file && log_file != stdout && log_file != stderr)
    {
        fclose(log_file);
        log_file = NULL;
    }

    pthread_mutex_unlock(&log_mutex);
    pthread_mutex_destroy(&log_mutex);
}

void logger_set_level(LogLevel level)
{
    if (level >= LOG_LEVEL_DEBUG && level <= LOG_LEVEL_NONE)
    {
        current_level = level;
    }
}

void logger_set_outputs(int outputs)
{
    current_outputs = outputs;
}

CQError logger_set_file(const char *filepath)
{
    if (!filepath)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    FILE *new_file = fopen(filepath, "a");
    if (!new_file)
    {
        return CQ_ERROR_FILE_NOT_FOUND;
    }

    pthread_mutex_lock(&log_mutex);

    if (log_file && log_file != stdout && log_file != stderr)
    {
        fclose(log_file);
    }

    log_file = new_file;

    pthread_mutex_unlock(&log_mutex);

    return CQ_SUCCESS;
}

static void log_message(LogLevel level, const char *format, va_list args)
{
    if (level < current_level)
    {
        return;
    }

    pthread_mutex_lock(&log_mutex);

    // Get current time
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buffer[20];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    // Format the message
    char message_buffer[1024];
    vsnprintf(message_buffer, sizeof(message_buffer), format, args);

    // Create the full log line
    char log_line[1150];
    snprintf(log_line, sizeof(log_line), "[%s] [%s] %s\n",
             time_buffer, level_strings[level], message_buffer);

    // Output to console if enabled
    if (current_outputs & LOG_OUTPUT_CONSOLE)
    {
        if (level >= LOG_LEVEL_WARNING)
        {
            fprintf(stderr, "%s[%s] %s%s", level_colors[level],
                    level_strings[level], message_buffer, color_reset);
        }
        else
        {
            printf("%s[%s] %s%s", level_colors[level],
                   level_strings[level], message_buffer, color_reset);
        }
    }

    // Output to file if enabled
    if ((current_outputs & LOG_OUTPUT_FILE) && log_file)
    {
        fprintf(log_file, "[%s] [%s] %s\n",
                time_buffer, level_strings[level], message_buffer);
        fflush(log_file);
    }

    pthread_mutex_unlock(&log_mutex);
}

void LOG_DEBUG(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_DEBUG, format, args);
    va_end(args);
}

void LOG_INFO(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_INFO, format, args);
    va_end(args);
}

void LOG_WARNING(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_WARNING, format, args);
    va_end(args);
}

void LOG_ERROR(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_ERROR, format, args);
    va_end(args);
}
