#include "libc.h"

void yield() {
  asm volatile( "svc #0     \n"  );
}

int write( int fd, void* x, size_t n ) {
  int r;

  asm volatile( "mov r0, %1 \n"
                "mov r1, %2 \n"
                "mov r2, %3 \n"
                "svc #1     \n"
                "mov %0, r0 \n" 
              : "=r" (r) 
              : "r" (fd), "r" (x), "r" (n) 
              : "r0", "r1", "r2" );

  return r;
}

int read(void* x){
  int r;

  asm volatile( "mov r0, %1 \n"
                "svc #2     \n"
                "mov %0, r0 \n" 
              : "=r" (r) 
              : "r" (x)
              : "r0" );

  return r;
}

int fork(int id){
  int r;

  asm volatile( "mov r0, %1 \n"
                "svc #3     \n"
                "mov %0, r0 \n" 
              : "=r" (r)
              : "r" (id)
              : "r0" );

  return r;
}

void exit(){
  asm volatile( "svc #4     \n");
}

int get_status(void* x){
  int r;

  asm volatile( "mov r0, %1 \n"
                "svc #5     \n"
                "mov %0, r0 \n" 
              : "=r" (r) 
              : "r" (x)
              : "r0" );

  return r;
}

void kill(int x){
  int r;
  asm volatile( "mov r0, %1 \n"
                "svc #6     \n"
              : "=r" (r) 
              : "r" (x) );

}


void printInt(int x){

  if(x == 0){
    write(0,"0",1);
  }else{

    int num = x;
    int size = 0;
    while(num>0){
      num /= 10;
      size++;
    }

    char c[size];
    num = x;
    for (int i = size - 1; i >= 0 ; i--)
    {
      c[i] = num % 10 + '0';
      num -= num % 10;
      num = num / 10;
    }
    write(0,&c[0],size);
  }

}