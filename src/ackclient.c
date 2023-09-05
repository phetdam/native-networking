/**
 * @file ackclient.c
 * @author Derek Huang
 * @brief Toy client for sending message to the toy acknowledgment server
 * @copyright MIT License
 *
 * This is based off of a heavily modified version of a toy client program by
 * Robert Ingalls which can be downloaded from the following link:
 *
 * https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/client.c
 */

// include first for platform detection macros
#include "pdnnet/platform.h"

#ifndef PDNNET_UNIX
#error "ackclient.c cannot be compiled for non-Unix platforms"
#endif  // PDNNET_UNIX

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "pdnnet/error.h"
#include "pdnnet/socket.h"

int
main(int argc, char **argv)
{
  int sockfd, portno, n;

  struct sockaddr_in serv_addr;
  struct hostent *server;

  char buffer[256];
  if (argc < 3) {
    fprintf(stderr,"usage %s hostname port\n", argv[0]);
    return EXIT_SUCCESS;
  }
  portno = atoi(argv[2]);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    PDNNET_ERRNO_EXIT(errno, "Failed to open socket");
  server = gethostbyname(argv[1]);
  if (!server)
    PDNNET_ERRNO_EXIT(errno, "No such host");
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
  serv_addr.sin_port = htons(portno);
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    PDNNET_ERRNO_EXIT(errno, "Could not connect to socket");
  printf("Please enter the message: ");
  memset(buffer, 0, sizeof buffer);
  // fgets reads sizeof buffer - 1 chars at max and adds a '\0'
  fgets(buffer, sizeof buffer, stdin);
  // don't write an extra newline if any
  size_t buf_size = strlen(buffer);
  if (buffer[buf_size - 1] == '\n')
    buffer[buf_size - 1] = '\0';
  n = write(sockfd, buffer, strlen(buffer));
  if (n < 0)
    PDNNET_ERRNO_EXIT(errno, "Socket write failed");
  // close write end to signal end of transmission
  if (shutdown(sockfd, SHUT_WR) < 0)
    PDNNET_ERRNO_EXIT(errno, "Shutdown with SHUT_WR failed");
  // read and print each received message chunk
  if (pdnnet_socket_fwrite(sockfd, stdout) < 0)
  {
    if (shutdown(sockfd, SHUT_RDWR) < 0)
      PDNNET_ERRNO_EXIT(errno, "Shutdown with SHUT_RDWR after read failed");
    PDNNET_ERRNO_EXIT(errno, "Read failed");
  }
  // trailing newline to finish off
  puts("");
  // done with the socket, so just close
  if (close(sockfd) < 0)
    PDNNET_ERRNO_EXIT(errno, "Failed to close socket");
  return EXIT_SUCCESS;
}
