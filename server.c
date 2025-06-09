#include <microhttpd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "bloom.h"

#define PORT 8080
#define MAX_POST_DATA_SIZE 1024

struct ConnectionInfo {
    char *data;
    size_t size;
};

void convert_key(const char* raw_key, const char* type, char* out_buf, size_t buf_size) {
    if (strcmp(type, "int") == 0) {
        int value = atoi(raw_key);
        snprintf(out_buf, buf_size, "%d", value);
    } else if (strcmp(type, "float") == 0) {
        float value = atof(raw_key);
        snprintf(out_buf, buf_size, "%.3f", value);
    } else {
        strncpy(out_buf, raw_key, buf_size - 1);
        out_buf[buf_size - 1] = '\0';
    }
}

enum MHD_Result answer_to_connection(void *cls,
                         struct MHD_Connection *connection,
                         const char *url,
                         const char *method,
                         const char *version,
                         const char *upload_data,
                         size_t *upload_data_size,
                         void **con_cls)
{
    if (0 != strcmp(method, "GET") && 0 != strcmp(method, "POST")) {
        return MHD_NO;
    }

    if (*con_cls == NULL) {
        struct ConnectionInfo *info = malloc(sizeof(struct ConnectionInfo));
        info->data = calloc(1, MAX_POST_DATA_SIZE);
        info->size = 0;
        *con_cls = info;
        return MHD_YES;
    }

    struct ConnectionInfo *info = *con_cls;

    if (*upload_data_size != 0) {
        size_t copy_size = (*upload_data_size > MAX_POST_DATA_SIZE - info->size) ?
                            (MAX_POST_DATA_SIZE - info->size) : *upload_data_size;
        memcpy(info->data + info->size, upload_data, copy_size);
        info->size += copy_size;
        *upload_data_size = 0;
        return MHD_YES;
    }

    const char *response_str = NULL;

    if (strcmp(method, "POST") == 0 && strcmp(url, "/add") == 0) {
        info->data[info->size] = '\0';
        char *key = strstr(info->data, "key=");
        char *type = strstr(info->data, "type=");
        if (key) {
            key += 4;
            char *end = strchr(key, '&');
            if (end) *end = '\0';

            char key_converted[128];
            convert_key(key, type ? type + 5 : "string", key_converted, sizeof(key_converted));
            bloom_add(key_converted);
            response_str = "Added";
        } else {
            response_str = "Missing key";
        }

    } else if (strcmp(method, "GET") == 0 && strncmp(url, "/check", 6) == 0) {
        const char *key = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "key");
        const char *type = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "type");

        if (key) {
            char key_converted[128];
            convert_key(key, type ? type : "string", key_converted, sizeof(key_converted));
            if (bloom_check(key_converted))
                response_str = "Probably in set";
            else
                response_str = "No";
        } else {
            response_str = "Missing key";
        }

    } else if (strcmp(method, "POST") == 0 && strcmp(url, "/init") == 0) {
        info->data[info->size] = '\0';
        char *hashes = strstr(info->data, "hashes=");
        if (hashes) {
            hashes += 7;
            if (bloom_configure(hashes))
                response_str = "Bloom filter initialized";
            else
                response_str = "Invalid hash function list";
        } else {
            response_str = "Missing hashes parameter";
        }

    } else if (strcmp(method, "GET") == 0 && strcmp(url, "/ping") == 0) {
        response_str = "pong";

    } else {
        response_str = "Invalid endpoint";
    }

    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(response_str),
        (void *)response_str,
        MHD_RESPMEM_PERSISTENT);
    enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    free(info->data);
    free(info);
    return ret;
}

volatile sig_atomic_t stop = 0;

void handler(int sig) {
    stop = 1;
}

int main() {
    signal(SIGINT, handler);
    signal(SIGTERM, handler);
    bloom_init();

    struct MHD_Daemon *daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT,
                                                 NULL, NULL,
                                                 &answer_to_connection, NULL,
                                                 MHD_OPTION_END);
    if (!daemon) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    printf("Server running on http://localhost:%d\n", PORT);
    while (!stop) {
        sleep(1);
    }

    MHD_stop_daemon(daemon);
    bloom_free();
    return 0;
}
