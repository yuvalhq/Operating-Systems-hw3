#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "message_slot.h"

#define ARG_NUM 3

int main(int argc, char const *argv[]) {
    int fd, ioctl_status;
    ssize_t message_len, write_status;
    char buffer[BUF_LEN];

    if (argc != ARG_NUM) {
        perror("Incorrect number of arguments\n");
        exit(ERROR);
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == FAIL) {
        perror("Failed to open device file\n");
        exit(ERROR);
    }

    ioctl_status = ioctl(fd, MSG_SLOT_CHANNEL, atoi(argv[2]));
    if (ioctl_status < 0){
        perror("Failed to set the channel id\n");
        exit(ERROR);
    }

    message_len = read(fd, buffer, BUF_LEN);
    if (message_len < 0) {
        perror("Failed to read from message slot file\n");
        exit(ERROR);
    }
    close(fd);

    write_status = write(1, buffer, message_len);
    if (write_status != message_len) {
        perror("Failed to write the message to stdout\n");
        exit(ERROR);
    }

    exit(SUCCESS);
}
