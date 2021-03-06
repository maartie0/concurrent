#include "P0.h"

int is_prime( uint32_t x ) {
  if ( !( x & 1 ) || ( x < 2 ) ) {
    return ( x == 2 );
  }

  for( uint32_t d = 3; ( d * d ) <= x ; d += 2 ) {
    if( !( x % d ) ) {
      return 0;
    }
  }

  return 1;
}

void P0() {
  int x = 0;
  char* y = "hello world, I'm P0\n";
  char* z = "is_prime( ";
  char* l = " )\n";

 while( 1 ) {
    // test whether each x for 2^8 < x < 2^24 is prime or not
    for( uint32_t x = ( 1 << 8 ); x < ( 1 << 24); x++ ) {
      int r = is_prime( x ); // printf( "is_prime( %d ) = %d\n", x, r );
      if(r == 1){
        write(0,z,10);
        printInt(x);
        write(0,l,3);
      }
    }
    //yield();
 }
    

  return;
}

void (*entry_P0)() = &P0;