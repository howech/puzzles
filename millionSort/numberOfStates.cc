
// Copyright (c) 2012
// Chris Howe
//
// All rights reserved for the moment. 

#include <cln/integer.h>
#include <cln/io.h>
#include <cln/integer_io.h>

#include <stdio.h>
#include <stdarg.h>

#include <execinfo.h>
#include <signal.h>

#include <exception>
#include <iostream>

#define BUCKETS 1000
#define MAX_INPUT 100000000
#define BUCKET_SIZE ( MAX_INPUT / BUCKETS )
#define LITTLE_BUCKET_MAX_SIZE 20

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

  return cln::exquo( number * c, (c+w-1) );
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

  return cln::exquo( number * (w-1), (c+w-1) );
}


cln::cl_I biggest= 0;
unsigned long biggest_count = 0;

cln::cl_I lookup[21];
bool lookup_init = false;

void optimizedFullWidthNumberOfStates( cln::cl_I &enc, unsigned long c ) {
  unsigned long cc;  

  if( !lookup_init) {
    for(unsigned long i = 0; i<21; ++i) {
      lookup[i] = numberOfStates(i,BUCKET_SIZE);
    }
  }

  if( biggest_count == 0) {
    biggest_count = c;
    biggest = numberOfStates( c, BUCKET_SIZE );
    enc = 0+biggest;
    return;
  }

  if( c > biggest_count ) {
    while( c>biggest_count) {
      biggest = advanceCount( biggest, biggest_count, BUCKET_SIZE);
      biggest_count ++;
    }
    enc = 0+biggest;
    return;
  }

  if( c > (biggest_count-20) / 2) {
    cc = biggest_count;
    enc = 0+biggest;
    while( cc > c ) {
      enc = retreatCount(enc,cc,BUCKET_SIZE);
      cc--;
    }
    return;
  }

  if( c > 20 ) {
    unsigned long c20 = 20;
    enc = 0+lookup[20];
    while( c > c20) {
      enc = advanceCount( enc, c20, BUCKET_SIZE);
      c20 ++;
    }
    return;
  }

  if( c <= 20 ) {
    enc = 0+lookup[c];
    return;
  }

  enc = 0+numberOfStates(c,BUCKET_SIZE);
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

  if( c > 0 ) {
    if(width == BUCKET_SIZE) {
      optimizedFullWidthNumberOfStates( separator, c-1 );
    } else {
    separator = numberOfStates( c-1, width);
    }
  }
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

void encode2( unsigned long number, cln::cl_I & encoding, cln::cl_I & sep, unsigned long & count, unsigned long min, unsigned long max ) 
{
  unsigned long width = max-min;
  unsigned long c = count;
  cln::cl_I separator = 0;
  cln::cl_I scratch = encoding;
  
  encoding = 0;

  if( sep > 0 ) {
    separator = 0+sep;
    sep = advanceCount( sep, c-1, width);
  } else  if( c > 0 ) {
    separator = numberOfStates( c-1, width);
    sep = 0+separator;
    sep = advanceCount( sep, c-1, width );
  } else {
    sep = numberOfStates(1,width);
  }

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


void merge_encode( cln::cl_I &enc1, unsigned long &count1, cln::cl_I & enc2, unsigned long & count2, unsigned long min, unsigned long max ) 
{
  unsigned long width = max-min;
  unsigned long c = count1+count2;
  unsigned long cres = c;
  cln::cl_I sep1 = 0;
  cln::cl_I sep2 = 0;
  cln::cl_I sepr = 0;
  cln::cl_I result = 0;


  if( c > 0 && count1 > 0 && count2 > 0 )
    sepr = numberOfStates( c-1, width);

  if(count1>0) 
    sep1 = numberOfStates( count1-1, width );

  if(count2>0) 
    sep2 = numberOfStates( count2-1, width );


  while(width > 0 && count1 > 0 && count2 > 0) {
    // merge in any numbers equal to min from 1
    while( enc1 < sep1 && count1 > 0) {
      sepr = retreatCount( sepr, c-1, width );
      c -= 1;
      sep1 = retreatCount( sep1, count1-1, width );
      count1 -= 1;
    }
    // merge in any numbers equal to min from 2
    while( enc2 < sep2 && count2 > 0) {
      sepr = retreatCount( sepr, c-1, width );
      c -= 1;
      sep2 = retreatCount( sep2, count2-1, width );
      count2 -= 1;
    }
    // that is all of the instances of min, so we advance
    result += sepr;
    sepr = retreatWidth( sepr, c-1, width);

    if(count1 > 0) {
      enc1 -= sep1;
      sep1 = retreatWidth( sep1, count1-1, width);
    }

    if(count2 > 0) {
      enc2 -= sep2;
      sep2 = retreatWidth( sep2, count2-1, width);
    }

    // move to next number
    min += 1;
    width -= 1;
  }

  // Whatever is left over in enc1 or enc2 can be stuffed back 
  // into the result
  if( count1 > 0 ) 
    result += enc1;
  if(count2 > 0)
    result += enc2;
  
  // return the resutl in enc1
  enc1 = result;
  count1 = cres;
  // return enc2 to empty
  enc2 = 0;
  count2 = 0;
} 


void optimized_merge_encode( cln::cl_I &enc1, unsigned long &count1, cln::cl_I & enc2, unsigned long & count2) 
{
  unsigned long width = BUCKET_SIZE;
  unsigned long c = count1+count2;
  unsigned long cres = c;
  cln::cl_I sep1 = 0;
  cln::cl_I sep2 = 0;
  cln::cl_I sepr = 0;
  cln::cl_I result = 0;

  if( c > 0 && count1 > 0 && count2 > 0 ) {
    optimizedFullWidthNumberOfStates( sepr, c-1);
  }
  //sepr = numberOfStates( c-1, width);

  if(count1>0) {
    optimizedFullWidthNumberOfStates( sep1, count1-1);
  }
  //sep1 = numberOfStates( count1-1, width );


  if(count2>0) { 
    optimizedFullWidthNumberOfStates( sep2, count2-1);
  }
  //sep2 = numberOfStates( count2-1, width );


  while(width > 0 && count1 > 0 && count2 > 0) {
    // merge in any numbers equal to min from 1
    while( enc1 < sep1 && count1 > 0) {
      sepr = retreatCount( sepr, c-1, width );
      c -= 1;
      sep1 = retreatCount( sep1, count1-1, width );
      count1 -= 1;
    }
    // merge in any numbers equal to min from 2
    while( enc2 < sep2 && count2 > 0) {
      sepr = retreatCount( sepr, c-1, width );
      c -= 1;
      sep2 = retreatCount( sep2, count2-1, width );
      count2 -= 1;
    }
    // that is all of the instances of min, so we advance
    result += sepr;
    sepr = retreatWidth( sepr, c-1, width);

    if(count1 > 0) {
      enc1 -= sep1;
      sep1 = retreatWidth( sep1, count1-1, width);
    }

    if(count2 > 0) {
      enc2 -= sep2;
      sep2 = retreatWidth( sep2, count2-1, width);
    }

    // move to next number
    //min += 1;
    width -= 1;
  }

  // Whatever is left over in enc1 or enc2 can be stuffed back 
  // into the result 
  if( count1 > 0 ) 
    result += enc1;
  if(count2 > 0)
    result += enc2;
  
  // return the resutl in enc1
  enc1 = 0+result;
  count1 = cres;
  // return enc2 to empty
  enc2 = 0;
  count2 = 0;
} 

void getBucketNumbers( unsigned long input, unsigned long &index, unsigned long &min, unsigned long &max) {
  index = input / BUCKET_SIZE;
  min = index * BUCKET_SIZE;
  max = min + BUCKET_SIZE;
}


unsigned long counts[BUCKETS];
cln::cl_I encodings[BUCKETS];
unsigned long little_counts[BUCKETS]; 
cln::cl_I little_encodings[BUCKETS];

void initialize() {
  for(int i=0;i<BUCKETS;++i) {
    counts[i] = 0;
    encodings[i] = 0;
    little_counts[i] = 0;
    little_encodings[i] = 0;
  }
}

void processInput( unsigned long input ) {
  unsigned long index, min, max;
  getBucketNumbers(input, index, min, max );
  if( index < BUCKETS ) {
    encode( input, little_encodings[index], little_counts[index], min, max);
    if( little_counts[index] >= LITTLE_BUCKET_MAX_SIZE ) {
      optimized_merge_encode(encodings[index], counts[index], little_encodings[index], little_counts[index]);
    }
  }
}


void processOutput() {
  unsigned long i, min, max;
  for( i=0; i< BUCKETS; ++i) {
    min = i * BUCKET_SIZE;
    max = min + BUCKET_SIZE;
    if( little_counts[i] > 0 )
      optimized_merge_encode(encodings[i], counts[i], little_encodings[i], little_counts[i]);
    if( counts[i] > 0 )
      decode( encodings[i], counts[i], min, max);
  }
}


int main() {
  for(unsigned long i = 0; i<100000; ++i ) {
    std::cout << i << " " << std::endl;
    processInput( cln::cl_I_to_ulong( cln::random_I( MAX_INPUT ) ) );
    //processInput( cln::cl_I_to_ulong( cln::random_I( 1000 ) ) );
  }
  
  std::cout << "Processing Output" << std::endl;
  
  processOutput();
}
