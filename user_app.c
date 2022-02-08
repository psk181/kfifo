#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define _IOC_WRITE     1U
#define MY_MACIG 'G'
#define DATA_READ _IOC(_IOC_WRITE,'K',0x0d,0x3FFF)

struct data_read {
        int buffer[32];
};

int main()
{
        struct data_read data;
        int len = 10, i;
        char *arr = (char *)malloc(200);
        int fd = open("/dev/pskdev", O_RDONLY);
        if (fd < 0) {
                printf("ERROR: %s\n", strerror(errno));
                return -1;
        }

#if 1   // Read command

        printf("Data Read\n");
        i = read(fd, arr, len);
        printf("Data read from Driver : %d\n", i);
        for(i = 0; i < len; i++)
        {
                printf("%c  ", arr[i]);
        }
        printf("\n");
        free(arr);
#endif

#if 0 // This is IOCTL example


        printf("IOCTL Sending.......... \n");
        int ret = ioctl(fd, DATA_READ, &data);
        if (ret < 0) {
                        printf("ERROR: %s", strerror(errno));
                        printf("ERROR in IOCTL REturn \n");
                        return -2;
                }

                for(i = 0; i < len-1; i++)
                        printf("Data : %d\n", data.buffer[i]);


#endif
        close(fd);
        return 0;
}

