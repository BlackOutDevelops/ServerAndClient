/* This is the server code */
#include "file-server.h"
#include <sys/fcntl.h>
#include <stdio_ext.h>

#define QUEUE_SIZE 10

void read_file(int sa, int fd, char *ip);
void write_file(int sa);



int main(int argc, char *argv[])
{
    int s, b, l, fd, sa, on = 1, flag;
    char ip[BUF_SIZE], fstring[BUF_SIZE], file[BUF_SIZE];
    struct sockaddr_in channel;		/* holds IP address */

    /* Build address structure to bind to socket. */
    memset(&channel, 0, sizeof(channel));	/* zero channel */
    channel.sin_family = AF_INET;
    channel.sin_addr.s_addr = htonl(INADDR_ANY);
    channel.sin_port = htons(SERVER_PORT);

    /* Passive open. Wait for connection. */
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);    /* create socket */
    if (s < 0) fatal("socket failed");
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));

    b = bind(s, (struct sockaddr *) &channel, sizeof(channel));
    if (b < 0) fatal("bind failed");

    l = listen(s, QUEUE_SIZE);		/* specify queue size */
    if (l < 0) fatal("listen failed");

    /* Socket is now set up and bound. Wait for connection and process it. */
    while (1)
    {
          sa = accept(s, 0, 0);		/* block for connection request */
          if (sa < 0) fatal("accept failed");

          read(sa, fstring, BUF_SIZE);
          flag = atoi(fstring);
          read(sa, ip, BUF_SIZE);

          if (flag == 1 || flag == 2)
              read_file(sa, fd, ip);
          else
              write_file(sa);
    }
}

void read_file(int sa, int fd, char *ip)
{
    int i, j, counter = 1, bytediff = 0, bytes, start_byte = 0, end_byte = 0, num_tenths;
    double tenth = 0;
    char buf[BUF_SIZE], newBuf[BUF_SIZE], filename[BUF_SIZE], start_byte_string[BUF_SIZE], end_byte_string[BUF_SIZE], error[BUF_SIZE]; /* buffer for outgoing file */

    memset(buf, 0, sizeof(buf));
    read(sa, buf, BUF_SIZE);		/* read file name from socket */

    start_byte = 0;
    end_byte = 0;
    if (strstr(buf, "txt") == NULL)
    {
        start_byte = atoi(buf);

        read(sa, end_byte_string, BUF_SIZE);

        end_byte = atoi(end_byte_string);

        read(sa, buf, BUF_SIZE);
    }

    if (access(buf, F_OK) == -1)
    {
      strcpy(error, "The file you requested does not exist in server directory.");
      write(sa, error, strlen(error));
    }

    /* Get and return the file. */
    fd = open(buf, O_RDONLY);	/* open the file to be sent back */
    if (fd < 0) fatal("open failed");

    i = 0;
    memset(filename, 0, sizeof(filename));
    while(buf[i] != '\0')
    {
        filename[i] = buf[i];
        i++;
    }

    memset(buf, 0, sizeof(buf));
    while (1) {
            bytes = read(fd, buf, BUF_SIZE);	/* read from file */
            if (bytes <= 0) break;		/* check for end of file */
            if (start_byte > bytes)
            {
                strcpy(error, "You are out of range with your starting bytes range.");
                write(sa, error, strlen(error));
                break;
            }
            else if (end_byte > bytes)
            {
                strcpy(error, "You are out of range with your ending bytes range.");
                write(sa, error, strlen(error));
                break;
            }
            else if (start_byte > end_byte)
            {
                strcpy(error, "Your starting byte range can not be greater than your ending byte range.");
                write(sa, error, strlen(error));
                break;
            }
            printf("Sending %s to %s\n", filename, ip);

            if (start_byte != 0 || end_byte != 0)
            {
                bytes = end_byte - start_byte;
                for (i = start_byte; i < end_byte; i++)
                {
                    newBuf[i - start_byte] = buf[i];
                }
            }
            tenth = (double)bytes/10;
            counter = 1;
            bytediff = 0;
            for (i = 0; counter < 11; i++)
            {
                if (((double)i/tenth) >= tenth && bytes < 10)
                {
                    num_tenths = i/tenth;
                    num_tenths -= (counter - 1);
                    for (j = 0; j < num_tenths; j++)
                    {
                        printf("Sent %d%% of %s\n", counter * 10, filename);
                        counter++;
                    }
                }
                else if (((double)i/tenth) > 1)
                {
                    num_tenths = i/tenth;
                    num_tenths -= (counter - 1);
                    for (j = 0; j < num_tenths; j++)
                    {
                        printf("Sent %d%% of %s\n", counter * 10, filename);
                        counter++;
                        bytediff += tenth;
                    }
                }

                if (i == bytes);
                else if (start_byte != 0 || end_byte != 0)
                    write(sa, newBuf + i, 1);		/* write bytes to socket */
                else
                    write(sa, buf + i, 1);		/* write bytes to socket */
              }

            printf("Finished sending %s to %s\n\n", filename, ip);
    }
    close(fd);			/* close file */
    close(sa);			/* close connection */
}

void write_file(int sa)
{
    int bytes, fd;
    char buf[BUF_SIZE], filename[BUF_SIZE], prompt[BUF_SIZE];

    memset(buf, 0, sizeof(buf));
    memset(filename, 0, sizeof(filename));
    read(sa, filename, BUF_SIZE);

    if (access(filename, F_OK) == 0)
    {
        strcpy(prompt, "The file you requested already exists server directory.");
        send(sa, prompt, strlen(prompt), 0);
        return;
    }
    else
    {
        strcpy(prompt, "Sending file...");
        send(sa, prompt, strlen(prompt), 0);
    }

    fd = creat(filename, O_RDWR);
    while (1) {
        bytes = read(sa, buf, BUF_SIZE);	/* read from socket */
        if (bytes <= 0)
        {
            break;
        }
        write(fd, buf, bytes);		/* write to standard output */
        strcpy(prompt, "File successfully sent.");
        send(sa, prompt, strlen(prompt), 0);
    }
    close(fd);
}
