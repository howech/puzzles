#include <cln/integer.h>
#include <cln/io.h>
#include <cln/integer_io.h>

#include <stdio.h>
#include <stdarg.h>

//  numberOfStates(n,m) = 
//                           ( n + m -1) !
//                          ---------------
//                            (m-1)! n!
//
// (Assuming m-1 > n)
//
//        n
//      -----   n + m - i
//       | |   -----------
//       | |        i
//      i = 1
//

cln::cl_I numberOfStates(unsigned long n, unsigned long m) {
  unsigned long i;
  unsigned long j = n+m-1;

  cln::cl_I result = 1;
  cln::cl_I ii;

  if( n > m-1 ) {
    n = m-1;
  }

  for(i=1; i<=n; ++i, --j) {
    ii = i;
    result *= j;
    result = cln::exquo(result, i) ;  // exquo(a,b) returns a/b if the result is an integer, throws an exception otherwise
  }

  return result;
}

cln::cl_I advanceCount( cln::cl_I number, unsigned long c, unsigned long w)
{
  //  (c+w-1)!           (c+w)!
  // ---------- --> ---------------
  // (w-1)! c!       (w-1)! (c+1)!

  return cln::exquo( number * (c+w) , (c+1) );
}

cln::cl_I retreatCount( cln::cl_I number, unsigned long c, unsigned long w)
{
  //  (c+w-1)!           (c+w-2)
  // ---------- --> ---------------
  // (w-1)! c!       (w-1)! (c-1)!

  return cln::exquo( number * c , (c+w-1) );
}


cln::cl_I advanceWidth( cln::cl_I number, unsigned long c, unsigned long w)
{
  //  (c+w-1)!          c+w
  // ---------- --> -----------
  // (w-1)! c!       (w)! (c)!

  return cln::exquo( number * (c+w) , w );
}

cln::cl_I retreatWidth( cln::cl_I number, unsigned long c, unsigned long w)
{
  //  (c+w-1)!          (c+w-2)
  // ---------- --> ---------------
  // (w-1)! c!       (w-2)! c!

  return cln::exquo( number * (w-1) , (c+w-1) );
}


void decode( cln::cl_I &encoded, unsigned long count, unsigned long min, unsigned long max )
{
  unsigned long width = max-min;

  // The empty set decode like melted butter
  if( count < 1)
    return;

  cln::cl_I separator = numberOfStates( count - 1, width );
  
  while( count > 0 && min < max ) {
    if( encoded < separator ) {
      std::cout << min << std::endl;
      if(count > 1)
	separator = retreatCount( separator, count-1, width );
      count = count-1;
    } else {
      encoded = encoded - separator;
      if(width > 1)
	separator = retreatWidth( separator, count-1, width );
      min += 1;
      width -= 1;
    }
  }
}


void encode( unsigned long number, cln::cl_I & encoding, unsigned long & count, unsigned long min, unsigned long max ) 
{
  unsigned long width = max-min;
  unsigned long c = count;
  cln::cl_I separator = 0;
  cln::cl_I scratch = encoding;
  
  encoding = 0;

  if( c > 0 )
    separator = numberOfStates( c-1, width);

  while( number > min && c>0 ) {        
    if( scratch < separator) {
      separator = retreatCount(separator, c-1, width );
      c = c-1;
    } else {
      scratch = scratch - separator;
      encoding += advanceCount( separator, c-1, width );
      separator = retreatWidth( separator, c-1, width );
      min += 1;
      width -= 1;
    }
  }

  if( number > min ) {
    encoding += numberOfStates( 1, number-min);
  } else {
    encoding += scratch;
  }
  count += 1;
} 

#define BUCKETS 1000
#define MAX_INPUT 100000000
#define BUCKET_SIZE ( MAX_INPUT / BUCKETS )

void getBucketNumbers( unsigned long input, unsigned long &index, unsigned long &min, unsigned long &max) {
  index = input / BUCKET_SIZE;
  min = index * BUCKET_SIZE;
  max = min + BUCKET_SIZE;
}


unsigned long counts[BUCKETS];
cln::cl_I encodings[BUCKETS];

void initialize() {
  for(int i=0;i<BUCKETS;++i) {
    counts[i] = 0;
    encodings[i] = 0;
  }
}

void processInput( unsigned long input ) {
  unsigned long index, min, max;
  getBucketNumbers(input, index, min, max );
  if( index < BUCKETS )
    encode( input, encodings[index], counts[index], min, max);
}


void processOutput() {
  unsigned long i, min, max;
  for( i=0; i< BUCKETS; ++i) {
    min = i * BUCKET_SIZE;
    max = min + BUCKET_SIZE;
    if( counts[i] > 0 )
      decode( encodings[i], counts[i], min, max);
  }
}


int main() {

  for(unsigned long i = 0; i<5000; ++i ) {
    processInput( cln::cl_I_to_ulong( cln::random_I( MAX_INPUT ) ) );
    processInput( cln::cl_I_to_ulong( cln::random_I( 1000 ) ) );
  }

  processOutput();

  return 0;
}
