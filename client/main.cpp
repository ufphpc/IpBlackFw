#include <stdio.h>
#include <stdlib.h>
#include <getopt.h> 	
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "interface_io.h"
//
int main(int argc, char **argv) {

    int ret = 1;

    do {
         if (argc != 3) {
            printf("Error: Invalid argc\n");
            break;
        }

        int fd = open(DRIVER_DEVICE_PATH, O_RDWR);
        if (fd < 0) {
            printf("Error: in opening /dev/fwDevice file : %d\n", errno);
            break;
        }

        struct in_addr ip_addr;

        ip_addr.s_addr = inet_addr(argv[2]);

        struct fw_set_info info;
        info.ip = ip_addr.s_addr;

        if(strcmp(argv[1], "-a") == 0) {

            if (ioctl(fd, FW_IOCTL_ADD_INFO, &info) < 0) {
                printf("Error: Call cmd FW_IOCTL_SET_INFO fail\n");
            } else {
                ret = 0;
            }

        } else if(strcmp(argv[1], "-d") == 0) {

            if (ioctl(fd, FW_IOCTL_DEL_INFO, &info) < 0) {
                printf("Error: Call cmd FW_IOCTL_DEL_INFO fail\n");
            } else {
                ret = 0;
            }

        } else {
            printf("Error: Invalid arg cmd\n");
        }

        close(fd);

    } while (false);

    return ret;
}






























