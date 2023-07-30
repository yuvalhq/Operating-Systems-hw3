#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "message_slot.h"

#define ARG_NUM 4

int main(int argc, char const *argv[]) {
    int fd, ioctl_status;
    size_t message_len;
    ssize_t write_status;
    
    if (argc != ARG_NUM) {
        perror("Incorrect number of arguments\n");
        exit(ERROR);
    }

    fd = open(argv[1], O_WRONLY);
    if (fd == FAIL) {
        perror("Failed to open the device file\n");
        exit(ERROR);
    }

    ioctl_status = ioctl(fd, MSG_SLOT_CHANNEL, atoi(argv[2]));
    if (ioctl_status < 0){
        perror("Failed to set the channel id\n");
        exit(ERROR);
    }

    message_len = strlen(argv[3]);
    write_status = write(fd, argv[3], message_len);
    if (write_status != message_len) {
        perror("Failed to write to the message slot file\n");
        exit(ERROR);
    }

    close(fd);
    exit(SUCCESS);
}