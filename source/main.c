#include <stdio.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "app.h"
#include "MTCEDEFs.h"
#include "mtce_video.h"

static int bRun = true;

void exit_handler()
{
    bRun = false;
}

int main(void)
{
    signal(SIGINT, exit_handler);
    signal(SIGQUIT, exit_handler);
    signal(SIGTERM, exit_handler);
    signal(SIGKILL, exit_handler);


    while (bRun)
    {

        usleep(1000);
    }

    printf("Realse main success\n");

    return 0;
}
