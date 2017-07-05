#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tunnel_server.h"

int main(int argc, char *argv[])
{
    if(argc<2){
        fprintf(stderr, "Usage: %s [port] [-v]\n", argv[0]);
        return -1;
    }

    int verbose = 0;
    if(argc>=3){
        if(strcmp(argv[2],"-v")==0){
            verbose = 1;
            fprintf(stdout,"verbose on\n");
        }
    }

    tunnel_server_start(argv[1],verbose);

    return 0;
}