#include "P1.h"

uint32_t gcd( uint32_t x, uint32_t y ) {
  if     ( x == y ) {
    return x;
  }
  else if( x >  y ) {
    return gcd( x - y, y );
  }
  else if( x <  y ) {
    return gcd( x, y - x );
  }
}

void P1() {
  while( 1 ) {
    // compute the gcd between pairs of x and y for 2^4 < x, y < 2^8
    char* y = "hello world, I'm P1\n";
    char* a = "gcd( ";
    char* b = ", ";
    char* c = " ) = ";
    char* d = "\n";

    write( 0, y, 20 );
    for( uint32_t x = ( 1 << 4 ); x < ( 1 << 6 ); x++ ) {
      for( uint32_t y = ( 1 << 4 ); y < ( 1 << 6 ); y++ ) {
        uint32_t r = gcd( x, y );  // printf( "gcd( %d, %d ) = %d\n", x, y, r );
        write(0,a,5);
        printInt(x);
        write(0,b,2);
        printInt(y);
        write(0,c,5);
        printInt(r);
        write(0,d,1);
      }
    }
    yield();
  }

  return;
}

void (*entry_P1)() = &P1;
