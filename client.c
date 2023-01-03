/* This page contains a client program that can request a file from the server program
 * on the next page. The server responds by sending the whole file.
 */

#include "file-server.h"
#include <sys/fcntl.h>

void retrieve_file(int s, char **argv);
void retrieve_byte_range_file(int s, char **argv);
void write_file(int s, char **argv);

int main(int argc, char **argv)
{
    int c, s, bytes, flag;
    char buf[BUF_SIZE], *ip, flagstr[BUF_SIZE];		/* buffer for incoming file */
    struct hostent *h;		/* info about server */
    struct sockaddr_in channel;		/* holds IP address */

    if (strstr(argv[2], "-w") != NULL)
        flag = 0;
    else if (strstr(argv[2], "-s") != NULL && strstr(argv[4], "-e") != NULL)
        flag = 1;
    else if (argc == 3)
        flag = 2;
    else
        flag = -1;

    if (flag == 2) printf("Usage:\n> For Byte Range Download: client server-name -s START_BYTE -e END_BYTE file-name\n\n"
                          "> For Writing a File: client server-name -w file-name\n\n");
    else if (flag == 1) printf("Usage:\n> For Full File Download: client server-name file-name\n\n"
                               "> For Writing a File: client server-name -w file-name\n\n");
    else if (flag == 0)   printf("Usage:\n> For Full File Download: client server-name file-name\n\n"
                                 "> For Byte Range Download: client server-name -s START_BYTE -e END_BYTE file-name\n\n");

    else fatal("Usage:\n> For Byte Range Download: client server-name -s START_BYTE -e END_BYTE file-name\n\n"
               "> For Full File Download: client server-name file-name\n\n"
               "> For Writing a File: client server-name -w file-name\n\n");
    h = gethostbyname(argv[1]);		/* look up host's IP address */
    if (!h) fatal("gethostbyname failed");

    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s <0) fatal("socket");
    memset(&channel, 0, sizeof(channel));
    channel.sin_family= AF_INET;
    memcpy(&channel.sin_addr.s_addr, h->h_addr, h->h_length);
    channel.sin_port= htons(SERVER_PORT);

    c = connect(s, (struct sockaddr *) &channel, sizeof(channel));
    if (c < 0) fatal("connect failed");

    flagstr[0] = flag + '0';
    write(s, flagstr, strlen(flagstr));
    sleep(1);

    ip = inet_ntoa(*((struct in_addr*) h->h_addr_list[0]));
    write(s, ip, strlen(ip));
    /* Connection is now established. Send file name including 0 byte at end. */

    if (flag == 2)
        retrieve_file(s, argv);
    else if (flag == 1)
        retrieve_byte_range_file(s, argv);
    else if (flag == 0)
        write_file(s, argv);


}

void retrieve_file(int s, char **argv)
{
    int bytes;
    char buf[BUF_SIZE];
    write(s, argv[2], strlen(argv[2])+1);

    /* Go get the file and write it to standard output. */
    while (1) {

          bytes = read(s, buf, BUF_SIZE);	/* read from socket */

          if (bytes <= 0)
          {
              printf("\n");
              exit(0);		/* check for end of file */
          }

          write(1, buf, bytes);		/* write to standard output */
    }
}

void retrieve_byte_range_file(int s, char **argv)
{
    int bytes;
    char buf[BUF_SIZE];

    write(s, argv[3], strlen(argv[3])+1);
    sleep(1);
    write(s, argv[5], strlen(argv[5])+1);
    write(s, argv[6], strlen(argv[6])+1);

    /* Go get the file and write it to standard output. */
    while (1) {

          bytes = read(s, buf, BUF_SIZE);	/* read from socket */

          if (bytes <= 0)
          {
              printf("\n");
              exit(0);		/* check for end of file */
          }

          write(1, buf, bytes);		/* write to standard output */
    }
}

void write_file(int s, char **argv)
{
    int fd, bytes;
    char buf[BUF_SIZE], server[BUF_SIZE];

    write(s, argv[3], strlen(argv[3])+1);
    recv(s, server, BUF_SIZE, 0);
    printf("%s\n", server);



    if (strstr(server, "Sending file...") == NULL)
        return;

    fd = open(argv[3], O_RDWR);
    if (fd < 0) fatal("File to be exported does not exist.");

    while (1)
    {
        bytes = read(fd, buf, BUF_SIZE);
        if (bytes <= 0)
        {
            break;
        }
        write(s, buf, bytes);
        memset(server, 0, sizeof(server));
        recv(s, server, BUF_SIZE, 0);
        printf("%s\n", server);
    }
    close(fd);
}
