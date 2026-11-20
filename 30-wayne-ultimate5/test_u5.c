#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#define WAYNE_IOC_MAGIC 'W'
#define WAYNE_IOC_PRESS _IO(WAYNE_IOC_MAGIC, 1)
#define WAYNE_IOC_RESET _IO(WAYNE_IOC_MAGIC, 2)
#define WAYNE_IOC_GET_COUNT _IOR(WAYNE_IOC_MAGIC, 3, int)
int main() {
    int fd = open("/dev/wayne_ultimate5", O_RDWR);
    if (fd < 0) { perror("open"); return 1; }
    int c = 0;
    ioctl(fd, WAYNE_IOC_PRESS);
    ioctl(fd, WAYNE_IOC_GET_COUNT, &c);
    printf("contador ioctl = %d\n", c);
    close(fd);
    return 0;
}
