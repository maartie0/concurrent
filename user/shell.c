#include "shell.h"

void clear_string(char* x);
void wait();
char array[50];
char status[50];

void wait(){
  for( uint32_t x = ( 1 << 8 ); x < ( 1 << 28); x++ )
  {
    asm volatile ( "nop     \n");
  }
}

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

void clear_string(char* x){
  for (int i = 0; i < 50; i++)
  {
    x[i] = '.';
  }
}


void shell() {
  
  //test();
  while(1){
    
    write(0,"\n> ",3);
    int length = read(&array);
    if(compare_strings(array,"p0")){
      int pid = fork(0);
      if(pid == -1){
        P0();
        exit();
      }else if(pid == -2){
        write(0,"can't add another process right now, please delete a running process\n",69);
        wait();
      }
    }
    if(compare_strings(array,"p1")){
      int pid = fork(1);
      if(pid == -1){
        P1();
        exit();
      }else if(pid == -2){
        write(0,"can't add another process right now, please delete a running process\n",69);
        wait();
      }
    }
    if(compare_strings(array,"p2")){
      int pid = fork(2);
      if(pid == -1){
        P2();
        exit();
      }else if(pid == -2){
        write(0,"can't add another process right now, please delete a running process\n",69);
        wait();
      }
    }
    if(compare_strings(array,"status")){
      int stat = get_status(status);
      for (int i = 0; i < stat; i++)
      {
        if(status[i] == '1'){
           write(0,"PROCESS ",8);
           printInt(i);
           write(0," IS RUNNING\n",12);
        }else{
           write(0,"PROCESS ",8);
           printInt(i);
           write(0," IS NOT RUNNING\n",16);
        }
      }
      wait();
      clear_string(status);
    }
    if(compare_strings(array,"kill process 0")){
        kill(0);
    }
    if(compare_strings(array,"kill process 1")){
        kill(1);
      }
    if(compare_strings(array,"kill process 2")){
        kill(2);
      }
    if(compare_strings(array,"kill process 3")){
        kill(3);
      }
    if(compare_strings(array,"kill process 4")){
        kill(4);
      }
    clear_string(array);
    yield();
  }
  
    
  
    
  return;
}




void (*entry_shell)() = &shell;