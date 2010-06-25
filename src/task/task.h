/* Copyright (c) 2005 Russ Cox, MIT; see COPYRIGHT */

#ifndef _TASK_H_
#define _TASK_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <inttypes.h>
#include <zmq.h>

/*
 * basic procs and threads
 */

typedef struct Task Task;
typedef struct Tasklist Tasklist;

int    anyready(void);
int    taskcreate(void (*f)(void *arg), void *arg, unsigned int stacksize);
void    taskexit(int);
void    taskexitall(int);
void    taskmain(int argc, char *argv[]);
int    taskyield(void);
void**    taskdata(void);
void    needstack(int);
void    taskname(char*, ...);
void    taskstate(char*, ...);
char*    taskgetname(void);
char*    taskgetstate(void);
void    tasksystem(void);
unsigned int  taskdelay(unsigned int);
unsigned int  taskid(void);

struct Tasklist  /* used internally */
{
  Task  *head;
  Task  *tail;
};

/*
 * queuing locks
 */
typedef struct QLock QLock;
struct QLock
{
  Task  *owner;
  Tasklist waiting;
};

void  qlock(QLock*);
int  canqlock(QLock*);
void  qunlock(QLock*);

/*
 * reader-writer locks
 */
typedef struct RWLock RWLock;
struct RWLock
{
  int  readers;
  Task  *writer;
  Tasklist rwaiting;
  Tasklist wwaiting;
};

void  rlock(RWLock*);
int  canrlock(RWLock*);
void  runlock(RWLock*);

void  wlock(RWLock*);
int  canwlock(RWLock*);
void  wunlock(RWLock*);

/*
 * sleep and wakeup (condition variables)
 */
typedef struct Rendez Rendez;

struct Rendez
{
  QLock  *l;
  Tasklist waiting;
};

void  tasksleep(Rendez*);
int  taskwakeup(Rendez*);
int  taskwakeupall(Rendez*);

/*
 * channel communication
 */
typedef struct Alt Alt;
typedef struct Altarray Altarray;
typedef struct Channel Channel;

enum
{
  CHANEND,
  CHANSND,
  CHANRCV,
  CHANNOP,
  CHANNOBLK,
};

struct Alt
{
  Channel    *c;
  void    *v;
  unsigned int  op;
  Task    *task;
  Alt    *xalt;
};

struct Altarray
{
  Alt    **a;
  unsigned int  n;
  unsigned int  m;
};

struct Channel
{
  unsigned int  bufsize;
  unsigned int  elemsize;
  unsigned char  *buf;
  unsigned int  nbuf;
  unsigned int  off;
  Altarray  asend;
  Altarray  arecv;
  char    *name;
};


/*
 * Threaded I/O.
 */
int fdread(int, void*, int);
int fdread1(int, void*, int);  /* always uses fdwait */
int fdrecv1(int, void*, int);  /* always uses fdwait */
int fdwrite(int, void*, int);
int fdsend(int, void*, int);
int fdrecv(int, void*, int);
int fdwait(int, int);
int fdnoblock(int);

void    fdtask(void*);

/*
 * 0mq Integration.
 */
void mqinit(int threads);
void *mqsocket(int type);
int mqwait(void *socket, int rw);
int mqrecv(void *socket, zmq_msg_t *msg, int flags);
int mqsend(void *socket, zmq_msg_t *msg, int flags);

/*
 * Network dialing - sets non-blocking automatically
 */
enum
{
  UDP = 0,
  TCP = 1,
};

int    netannounce(int, char*, int);
int    netaccept(int, char*, int*);
int    netdial(int, char*, int);
int    netlookup(char*, uint32_t*);  /* blocks entire program! */
int    netdial(int, char*, int);

#ifdef __cplusplus
}
#endif
#endif
