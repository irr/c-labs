/*
 * tcpclient.c
 *
 * Example of BSD TCP sockets client application.
 * Paired with tcpserver.c (BSD tcp server).
 *
 * OS: SunOS
 * Compiler: cc
 *	to compile: % cc -o tcpclient -g tcpclient.c
 * syntax: tcpclient remotehost
 *
 *	The server must be running.
 *	% tcpclient localhost
 *	% tcpclient sirius.cs.pdx.edu
 *
 * The TCP port is set with a #define, see TCPPORT below.
 * NOREQUESTS (10) buffers of size BUFSIZE (1024) are sent to
 *	the server.  The server reads the packets and writes them
 *	back to the client.
 *
 * compiler note:
 *	This is K&R C, not ANSI C.  The "cc" above is the C compiler
 *	as found on a BSD UNIX system (Say SunOS 4.1.4).  You should
 *	probably use gcc on other systems.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/times.h>

#define TRUE 1
#define FALSE 0

#define TCPPORT 	10000	/* shared client/server tcp port number */
#define BUFSIZE 	1024	/* size of buffer sent */
#define NOREQUESTS 	10	/* no of buffer's sent */


/* main i/o buffer
*/
static char iobuf[BUFSIZE];

/*
 * main
 *	
 *	fireup socket
 *	connect to remote server
 *	doit - do i/o to remote server
 *	close socket
 */
main(argc,argv)
int argc;
char **argv;
{
	int sock;
	struct sockaddr_in server;
	char *remote;

	fillBuf( iobuf, BUFSIZE);
	setbuf(stdout, NULL);

	if ( argc != 2 ) {
		fprintf(stderr,"tcpclient remote\n");
		exit(1);
	}
	remote = argv[1];
	printf("remote %s, \n",remote);

	/*
 	 * fireup socket and setup server structure
	 * for connect
	 */
	sock = initSocket(&server, remote);

	/* connect to remote server
	*/
	if (connect(sock, &server, sizeof(server)) < 0 ) {
		perror("connecting stream socket");
		exit(1);
	}

	doit(sock,iobuf);

	close(sock);

	printf("tcpclient finished ok\n");
	return(0);
}

/* do read write on socket to remote server.
 *
 * only read/write calls here
 */
doit(sock,buf) 
int sock;
char *buf;
{ 

	/* echo tests. we write packet to remote.
	 *	       we then read packet back.
	 */
	echoTest(sock,buf);
}

/* put data (readable ASCII) in buffer
*/
fillBuf(buf,size)
char *buf;
int size;
{
	char *spt;
	int len;
	int i;
	char current;

	current = 'a';
	spt = buf;

	/* loop through buffer of "size", if
         * chars mod 64, put in newline and set
	 * current to next letter
	 */
	for ( i = 0; i < size; i++) {
		if ( (i % 64) == 0 ){
			*spt++ = '\n';
			if ( i != 0 )
				current++;
			continue;
		}
		*spt++ = current;
	}
}

/*
 * write buffer to remote and read same amount back
 */
echoTest(sock,buf)
int sock;
char *buf;
{
	register int i;
	int rc;
	unsigned int writecsum,readcsum;
	int noerrs;

	noerrs = 0;
	/* for NOREQUESTS write/read iterations
	*/
	for ( i = 0; i < NOREQUESTS; i++) {
#ifdef DEBUG
		printf("client: iteration %d, about to write\n", i);
#endif
		if ( (rc = write(sock, buf, BUFSIZE)) < 0 ) {
			perror("client: writing on socket stream");
			exit(1);
		}
		/* read data back from socket
		*/
		doRead(sock, buf, BUFSIZE);
#ifdef DEBUG
		printf("client: iteration %d, readback complete\n", i);
#endif
		/* more debug. print data to stdout
		write(1,buf,BUFSIZE);
		*/
	}
}


/* read from socket. TCP read call issued to kernel
 * may return with less data than you expected. This routine
 * must check that and force a complete read.
 */
doRead(sock, buf,amountNeeded)
int sock;
char *buf;
int amountNeeded;
{
	register int i;
	int rc;
	char *bpt;
	int count = amountNeeded;
	int amtread;

	bpt = buf;
	amtread = 0;
 
again:
	if ( (rc = read(sock,bpt,count)) < 0 ) {
		perror("doRead: reading socket stream");
		exit(1);
	}
	amtread += rc;

	if ( amtread < amountNeeded ) {
		count = count - rc;	
		bpt = bpt + rc;
		goto again;
	}
}

/* 
 * dump a buffer in hex
 */
printBuf(buf,size)
char *buf;
{
	int i;
	i = 16;
	while(size--) {
		printf("%x", (int)(*buf++&0xff));
		if ( i == 0 ) {
			i = 16;
			printf("\n");
		}
		else
			i--;
	}
}

/*
 * initSocket
 *	create a tcp socket
 *	fill in the server structure with
 *		protocol family, remote address, tcp port
 * CALLS:
 *	gethostbyname - map hostname to host addr
 */
static
initSocket(server, remote)
struct sockaddr_in *server; 	/* server structure for connect */
char *remote;		        /* remote hostname */
{
	struct hostent *gethostbyname();
	struct hostent *h;
	int sock;

	/* create INTERNET,TCP socket
	*/
	sock = socket(AF_INET, SOCK_STREAM, 0); 

	if ( sock < 0 ) {
		perror("socket");
		exit(1);
	}

	/* fill in the server structure for the connect call
	 *
	 * family: INET meaning tcp/ip/ethernet.
	 * addr: string that represents INET number for remote system.
	 * port: string that represents tcp server number.
	 */ 
	server->sin_family = AF_INET;
	h = 0;

	/* read local inet name of REMOTE machine into 
	 * hostent structure. This buffer must be copied into the
	 * sockaddr_in server structure.
	 */
	h = gethostbyname(remote);
	if ( h == 0 ) {
		fprintf(stderr,"gethostbyname: cannot find host %s\n",remote);
		exit(2);
	}

	/*
	bcopy(h->h_addr, &server->sin_addr, h->h_length);
	*/
	memcpy(&server->sin_addr, h->h_addr, h->h_length);

	/* convert host short to network byteorder short
	*/
	server->sin_port = htons(TCPPORT);
	return(sock);
}

