#include <microhttpd.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define PORT 8080

volatile sig_atomic_t stop = 0;

enum MHD_Result answer_to_connection(void *cls,
                         struct MHD_Connection *connection,
                         const char *url,
                         const char *method,
                         const char *version,
                         const char *upload_data,
                         size_t *upload_data_size,
                         void **con_cls)
{
    const char *response_str;

    if (strcmp(method, "GET") != 0) {
        return MHD_NO;
    }

    if (strcmp(url, "/ping") == 0) {
        response_str = "pong";
    } else {
        response_str = "Not found";
    }

    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(response_str),
        (void *)response_str,
        MHD_RESPMEM_PERSISTENT);

    int status_code = strcmp(response_str, "pong") == 0 ? MHD_HTTP_OK : MHD_HTTP_NOT_FOUND;

    enum MHD_Result ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    return ret;
}

void handle_signal(int signum)
{
    printf("\nSignal %d received, stopping...\n", signum);
    stop = 1;
}

int main()
{
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT,
                              NULL, NULL,
                              &answer_to_connection, NULL,
                              MHD_OPTION_END);

    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    printf("Server running on http://localhost:%d\n", PORT);

    while (!stop) {
        sleep(1);
    }

    MHD_stop_daemon(daemon);
    printf("Server stopped\n");
    return 0;
}