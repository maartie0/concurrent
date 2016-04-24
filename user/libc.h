#ifndef __LIBC_H
#define __LIBC_H

#include <stddef.h>
#include <stdint.h>

// cooperatively yield control of processor, i.e., invoke the scheduler
void yield();

// write n bytes from x to the file descriptor fd
int write( int fd, void* x, size_t n );

int read(void* x);

void printInt(int x);

int fork(int id);

void exit();

void kill(int x);

void P0();
void P1();
void P2();
void idle();
int get_status(void* x);

#endif
