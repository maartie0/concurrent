#include "shell.h"

int compare_strings(char* x,char* y){
  if (x[0] == '\r') return 0;
  int i = 0;
  while (x[i] != '\r') {
    if (y[i] == '\0' )    return 0;
    if (x[i] == '\0' ) return 0;
    if (x[i] != y[i])  return 0;
    i++;
  }
  if(y[i] != '\0') return 0;

  return 1;
}

void test(){
  int b = compare_strings("ab","ab\r");
  int c = compare_strings("wbvjhbrwj","wlebfjkvwbfweb jv");
  int d = compare_strings("abc","abcd");
  int e = compare_strings("abcd","abc");
  int f = compare_strings("abc\n","abc\r");
  int g = compare_strings("abc\r","abc\r");
  int h = compare_strings("abc","abc\r");

  if(h  && b && f && g){
    write(0,"the yes cases are ok",20);
  }else{
    write(0,"yes cases failed",16);
  }
  if(c || d || e){
    write(0,"a string got through the test",29);
  }else{
    write(0,"fail cases are good",19);
  }

}


void shell() {
  
  //test();
  while(1){
    char array[50];
    int length = read(&array);
    if(compare_strings(array,"idle")){
      int pid = fork();
      if(pid == -1){
        while(1){
          idle();
        }
      }
    }
    if(compare_strings(array,"p0")){
      int pid = fork();
      if(pid == -1){
        P0();
        // exit();
      }
    }
    if(compare_strings(array,"p1")){
      int pid = fork();
      if(pid == -1){
        P1();
        // exit();
      }
    }
    if(compare_strings(array,"p2")){
      int pid = fork();
      if(pid == -1){
        P2();
        // exit();
      }
    }
    yield(); 
  }
    
  
    
  return;
}




void (*entry_shell)() = &shell;