#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define QUEUE_LENGTH 20
#define GET_REQUEST_SIZE 2048
#define GET_RESPONSE_HEADER 2048
#define RESULT_HEADER_SIZE 1024
#define ERROR_MESSAGE_SIZE 1024
#define FAILURE 2
int master_fd = -1;
pthread_mutex_t accept_con_mutex = PTHREAD_MUTEX_INITIALIZER;

/**********************************************
 * init
   - port is the number of the port you want the server to be
     started on
   - initializes the connection acception/handling system
   - if init encounters any errors, it will call exit().
************************************************/
void init(int port) {
  // Set up socket and set it to a reusable socket
  // Exit if any error occurs during any of the steps
  int sd;
  struct sockaddr_in addr;
  int ret_val;
  int flag;

  /**********************************************
   * IMPORTANT!
   * ALL TODOS FOR THIS FUNCTION MUST BE COMPLETED FOR THE INTERIM
   *SUBMISSION!!!!
   **********************************************/

  // TODO: Create a socket and save the file descriptor to sd (declared above)
  // This socket should be for use with IPv4 and for a TCP connection.
  sd = socket(PF_INET, SOCK_STREAM, 0);
  if (sd < 0) {
    perror("Socket creation failed \n");
    exit(EXIT_FAILURE);
  }

  // TODO: Change the socket options to be reusable using setsockopt().
  flag = 1;
  ret_val =
      setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(int));
  if (ret_val < 0) {
    perror("Unable to make socket reusable \n");
    exit(EXIT_FAILURE);
  }
  // TODO: Bind the socket to the provided port.
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("Unable to bind the socket\n");
    exit(EXIT_FAILURE);
  }
  // TODO: Mark the socket as a pasive socket. (ie: a socket that will be used
  // to receive connections)
  if (listen(sd, QUEUE_LENGTH) == -1) {
    perror("Failed to mark the socket as a passive socket\n");
    exit(EXIT_FAILURE);
  }

  // We save the file descriptor to a global variable so that we can use it in
  // accept_connection().
  master_fd = sd;
  printf("UTILS.O: Server Started on Port %d\n", port);
}

/**********************************************
 * accept_connection - takes no parameters
   - returns a file descriptor for further request processing.
     DO NOT use the file descriptor on your own -- use
     get_request() instead.
   - if the return value is negative, the thread calling
     accept_connection must should ignore request.
***********************************************/
int accept_connection(void) {
  int newsock;
  struct sockaddr_in new_recv_addr;
  uint addr_len;
  addr_len = sizeof(new_recv_addr);

  /**********************************************
   * IMPORTANT!
   * ALL TODOS FOR THIS FUNCTION MUST BE COMPLETED FOR THE INTERIM
   *SUBMISSION!!!!
   **********************************************/

  // TODO: Aquire the mutex lock
  if (pthread_mutex_lock(&accept_con_mutex) < 0) {
    perror("Failed to acquire connection lock");
    exit(EXIT_FAILURE);
  }
  // TODO: Accept a new connection on the passive socket and save the fd to
  // newsock
  newsock = accept(master_fd, (struct sockaddr *)&new_recv_addr, &addr_len);
  // TODO: Release the mutex lock
  if (pthread_mutex_unlock(&accept_con_mutex) < 0) {
    perror("Failed to unlock the mutex");
    exit(EXIT_FAILURE);
  }

  // TODO: Return the file descriptor for the new client connection

  return newsock;
}

/**********************************************
 * get_request
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        from where you wish to get a request
      - filename is the location of a character buffer in which
        this function should store the requested filename. (Buffer
        should be of size 1024 bytes.)
   - returns 0 on success, nonzero on failure. You must account
     for failures because some connections might send faulty
     requests. This is a recoverable error - you must not exit
     inside the thread that called get_request. After an error, you
     must NOT use a return_request or return_error function for that
     specific 'connection'.
************************************************/
int get_request(int fd, char *filename) {

  /**********************************************
   * IMPORTANT!
   * THIS FUNCTION DOES NOT NEED TO BE COMPLETE FOR THE INTERIM SUBMISSION, BUT
   *YOU WILL NEED CODE IN IT FOR THE INTERIM SUBMISSION!!!!!
   **********************************************/

  char buf[GET_REQUEST_SIZE];

  // INTERIM TODO: Read the request from the file descriptor into the buffer
  FILE *filestream = fdopen(fd, "r");
  if (filestream == NULL) {
    perror("Failed to open the connection: \n");
    return FAILURE;
  }
  // HINT: Attempt to read 2048 bytes from the file descriptor.
  if (fgets(buf, GET_REQUEST_SIZE, filestream) == NULL) {
    perror("Failed to read ");
    return FAILURE;
  }
  // INTERIM TODO: Print the first line of the request to the terminal.

  // TODO: Ensure that the incoming request is a properly formatted HTTP "GET"
  // request The first line of the request must be of the form: GET <file name>
  // HTTP/1.0 or: GET <file name> HTTP/1.1 HINT: It is recommended that you look
  // up C string functions such as sscanf and strtok for help with parsing the
  // request.
  // TODO: Extract the file name from the request
  char get[GET_REQUEST_SIZE], fname[GET_REQUEST_SIZE], http[GET_REQUEST_SIZE];
  sscanf(buf, "%s %s %s", get, fname, http);
  if (strcmp(get, "GET") != 0) {
    perror("GET in the incoming request is not properly formatted\n");
    return FAILURE;
  }
  if (!((strcmp(http, "HTTP/1.0") == 0) || (strcmp(http, "HTTP/1.1") == 0))) {
    perror("HTTP in the incoming request is not properly formatted\n");
    return FAILURE;
  }

  // TODO: Ensure the file name does not contain with ".." or "//"
  // FILE NAMES WHICH CONTAIN ".." OR "//" ARE A SECURITY THREAT AND MUST NOT BE
  // ACCEPTED!!! HINT: It is recommended that you look up the strstr function
  // for help looking for faulty file names.
  char *dotdot, *dashdash;
  dotdot = strstr(fname, "..");
  dashdash = strstr(fname, "//");
  if ((dotdot != NULL) || (dashdash != NULL)) {
    perror("File name contains \"..\" or \"//\" which is a security threat\n");
    return FAILURE;
  }

  // TODO: Copy the file name to the provided buffer
  strncpy(filename, fname, GET_REQUEST_SIZE);

  return 0;
}

/**********************************************
 * return_result
   - returns the contents of a file to the requesting client
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        to where you wish to return the result of a request
      - content_type is a pointer to a string that indicates the
        type of content being returned. possible types include
        "text/html", "text/plain", "image/gif", "image/jpeg" cor-
        responding to .html, .txt, .gif, .jpg files.
      - buf is a pointer to a memory location where the requested
        file has been read into memory (the heap). return_result
        will use this memory location to return the result to the
        user. (remember to use -D_REENTRANT for CFLAGS.) you may
        safely deallocate the memory after the call to
        return_result (if it will not be cached).
      - numbytes is the number of bytes the file takes up in buf
   - returns 0 on success, nonzero on failure.
************************************************/
int return_result(int fd, char *content_type, char *buf, int numbytes) {

  // TODO: Prepare the headers for the response you will send to the client.
  // REQUIRED: The first line must be "HTTP/1.0 200 OK"
  // REQUIRED: Must send a line with the header "Content-Length: <file length>"
  // REQUIRED: Must send a line with the header "Content-Type: <content type>"
  // REQUIRED: Must send a line with the header "Connection: Close"

  // NOTE: The items above in angle-brackes <> are placeholders. The file length
  // should be a number and the content type is a string which is passed to the
  // function.

  /* EXAMPLE HTTP RESPONSE
   *
   * HTTP/1.0 200 OK
   * Content-Length: <content length>
   * Content-Type: <content type>
   * Connection: Close
   *
   * <File contents>
   */
  char buf_header[RESULT_HEADER_SIZE];
  sprintf(buf_header,
          "HTTP/1.0 200 OK\nContent-Length: %d\nContent-Type: %s\nConnection: "
          "Close\n\n",
          numbytes, content_type);

  // TODO: Send the HTTP headers to the client
  // IMPORTANT: Add an extra new-line to the end. There must be an empty line
  // between the headers and the file contents, as in the example above.
  if (write(fd, buf_header, RESULT_HEADER_SIZE) < 0) {
    perror("Failed to write to stream");
    close(fd);
    return FAILURE;
  }

  // TODO: Send the file contents to the client
  if (write(fd, buf, numbytes) < 0) {
    perror("Write of file contents failed");
    close(fd);
    return FAILURE;
  }

  // TODO: Close the connection to the client
  close(fd);

  return 0;
}

/**********************************************
 * return_error
   - returns an error message in response to a bad request
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        to where you wish to return the error
      - buf is a pointer to the location of the error text
   - returns 0 on success, nonzero on failure.
************************************************/
int return_error(int fd, char *buf) {

  // TODO: Prepare the headers to send to the client
  // REQUIRED: First line must be "HTTP/1.0 404 Not Found"
  // REQUIRED: Must send a header with the line: "Content-Length: <content
  // length>" REQUIRED: Must send a header with the line: "Connection: Close"
  // NOTE: In this case, the content is what is passed to you in the argument
  // "buf". This represents a server generated error message for the user. The
  // length of that message should be the content-length. IMPORTANT: Similar to
  // sending a file, there must be a blank line between the headers and the
  // content.
  /* EXAMPLE HTTP ERROR RESPONSE
   *
   * HTTP/1.0 404 Not Found
   * Content-Length: <content length>
   * Connection: Close
   *
   * <Error Message>
   */
  char buf_header[ERROR_MESSAGE_SIZE];
  sprintf(buf_header,
          "HTTP/1.0 404 Not Found\nContent-Length: %ld\nConnection: Close\n\n",
          strlen(buf));

  // TODO: Send headers to the client
  if (write(fd, buf_header, ERROR_MESSAGE_SIZE) < 0) {
    perror("Write failure");
    close(fd);
    return FAILURE;
  }

  // TODO: Send the error message to the client
  if (write(fd, buf, strlen(buf)) < 0) {
    perror("Write failure");
    close(fd);
    return FAILURE;
  }

  // TODO: Close the connection with the client.
  close(fd);

  return 0;
}
