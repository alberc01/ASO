#ifndef _MENSAJE_H
#define _MENSAJE_H

#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>

struct _dato_t
{
    long tipo;
    char mensaje[80];
};

typedef struct _dato_t dato_t;    

struct _msg_t
{
    dato_t dato;
};
typedef struct _msg_t msg_t;

#define LOG_FILE "time.log"

#define LONG (sizeof(msg_t) - sizeof (long))

#define LIST	1
#define ADD	2
#define FIN     3
#define MAX	100000

#define TOK_CS	'a'
#define TOK_SC	'b'
#define KEYFILE "./mensaje.h"

#endif
