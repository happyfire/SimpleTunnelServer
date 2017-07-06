#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "tunnel_server.h"
#include "utils.h"

static const char *short_options = "hp:v:";
static const struct option long_options[] = {
        {"port", required_argument, NULL, 'p'},
        {"verbose", required_argument, NULL, 'v'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
};

int main(int argc, char *argv[])
{
    char *port = "4000";
    g_verbose = 0;

    int opt = 0;
    while( (opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1){
        switch (opt){
            case 'h':
            case '?':
                fprintf(stdout, "Usage: %s --port|-p {port} --verbose|-v {verbose_level}\n", argv[0]);
                return 0;
            case 'p':
                port = optarg;
                break;
            case 'v':
                g_verbose = atoi(optarg);
                break;
        }
    }

    LOG("server starting at port %s, verbose = %d",port,g_verbose);

    tunnel_server_start(port);

    return 0;
}