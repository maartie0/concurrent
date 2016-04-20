#include "shell.h"

int compare_strings(char* x,char* y){
  if(x[0] == '\r') return 0;
  int i = 0;
  while(x[i] != '\0'){
    if(x[i] != y[i]) return 0;
    if(y[i] == '\0') return 0;
    i++;
  }
  if(y[i] != '\r') return 0;
    
  return 1;
}


void shell() {
  char array[50];
  while(1){
    int length = read(array);
    if(compare_strings("p0",array)){
      int volatile pid = fork();
      write(0,"\n\n\n\n\n\n\n\n\n\n",10);
      if(pid == -1){
        while(1){
          P1();
        }
      }else{
          P0();
      }
    }
    if(compare_strings("p1",array)){
      int pid = fork();
      if(pid == 0){
        while(1){
          P2();
        }
      }
    }
    yield();
  }
    
    
    
  return;
}


void (*entry_shell)() = &shell;