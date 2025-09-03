#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ui/progress_display.h"
#include "utils/logger.h"

static char current_title[256];
static int total_items = 0;
static int current_item = 0;
static clock_t start_time;

CQError progress_display_init(void) {
    memset(current_title, 0, sizeof(current_title));
    total_items = 0;
    current_item = 0;

    LOG_INFO("Progress display initialized");
    return CQ_SUCCESS;
}

void progress_display_shutdown(void) {
    LOG_INFO("Progress display shutdown");
}

void progress_start(const char* title, int items) {
    if (!title) {
        LOG_ERROR("Invalid progress title");
        return;
    }

    strcpy(current_title, title);
    total_items = items;
    current_item = 0;
    start_time = clock();

    printf("\n%s\n", title);
    printf("Progress: [");
    for (int i = 0; i < 50; i++) {
        printf(" ");
    }
    printf("] 0%%\n");

    LOG_INFO("Started progress tracking: %s (%d items)", title, items);
}

void progress_update(int item, const char* status) {
    current_item = item;

    if (total_items <= 0) {
        return;
    }

    // Calculate progress percentage
    float progress = (float)current_item / (float)total_items;
    int percent = (int)(progress * 100.0f);

    // Calculate elapsed time and ETA
    clock_t current_time = clock();
    double elapsed = (double)(current_time - start_time) / CLOCKS_PER_SEC;
    double eta = (progress > 0.0f) ? (elapsed / progress - elapsed) : 0.0;

    // Update progress bar
    printf("\rProgress: [");
    int filled = (int)(progress * 50.0f);
    for (int i = 0; i < 50; i++) {
        if (i < filled) {
            printf("=");
        } else if (i == filled) {
            printf(">");
        } else {
            printf(" ");
        }
    }
    printf("] %d%% (%d/%d)", percent, current_item, total_items);

    if (status && strlen(status) > 0) {
        printf(" - %s", status);
    }

    if (eta > 0.0) {
        printf(" ETA: %.1fs", eta);
    }

    fflush(stdout);

    // Log progress periodically
    static int last_logged_percent = -1;
    if (percent >= last_logged_percent + 10 || percent == 100) {
        LOG_INFO("Progress: %d%% (%d/%d)", percent, current_item, total_items);
        last_logged_percent = percent;
    }
}

void progress_complete(const char* message) {
    // Complete the progress bar
    printf("\rProgress: [");
    for (int i = 0; i < 50; i++) {
        printf("=");
    }
    printf("] 100%% (%d/%d)", total_items, total_items);

    if (message && strlen(message) > 0) {
        printf(" - %s", message);
    }

    printf("\n");

    // Calculate total time
    clock_t end_time = clock();
    double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Completed in %.2f seconds\n\n", total_time);

    LOG_INFO("Progress completed: %s (%.2fs)", message ? message : "Done", total_time);

    // Reset state
    memset(current_title, 0, sizeof(current_title));
    total_items = 0;
    current_item = 0;
}

void progress_display_error(const char* message) {
    if (!message) {
        return;
    }

    fprintf(stderr, "ERROR: %s\n", message);
    LOG_ERROR("%s", message);
}

void progress_display_warning(const char* message) {
    if (!message) {
        return;
    }

    printf("WARNING: %s\n", message);
    LOG_WARNING("%s", message);
}

void progress_display_info(const char* message) {
    if (!message) {
        return;
    }

    printf("INFO: %s\n", message);
    LOG_INFO("%s", message);
}
