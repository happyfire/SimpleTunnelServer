#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tunnel_server.h"
#include "utils.h"

int main(int argc, char *argv[])
{
    if(argc<2){
        fprintf(stderr, "Usage: %s [port] [-v]\n", argv[0]);
        return -1;
    }

    if(argc>=3){
        if(strcmp(argv[2],"-v")==0){
            g_verbose = 3;
            LOG("verbose level:%d",g_verbose);
        }
    }

    tunnel_server_start(argv[1]);

    return 0;
}