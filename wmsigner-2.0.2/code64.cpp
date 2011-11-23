/****************************************************************************************************************************
 *  Base64 Encoder / Decoder module. Light Version 1.0
 *  Written by Georgy Mushkarev (c) for wmsigner Library. Moscow, Russia, 1992-2007.
 ****************************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"

/*  Main functions */

int main( int argc, char *argv[] );
void fatal_error( char *msg );

void fatal_error( char *msg )
{
  fprintf( stderr, "%s", msg);
  exit(2);
}


int main( int argc, char *argv[] )
{
  FILE *inputfile;
  size_t Counter = 0;
  size_t Bytes = 0;
  size_t i = 0;
  
  char buffer_inp[MAXBUFFER];
  char buffer_out[MAXBUFFER];

  if( argc == 1 || strcmp( argv[1], "-h" ) == 0 || strcmp( argv[1], "--help") == 0 ) {
    printf( "Program for Encode / Decode input File in Base64 system. Version 1.0\n\r");
    printf( "Usage to encode to Base64: code64 -K64 [--key64] Input_File [> Output_File]\n\r");
    printf( "Usage to decode to Bytes : code64 -E64 [--enc64] Input_File [> Output_File]\n\r");
    printf( "Author: Georgy Mushkarev (c), Moscow 2007\n\r");
    exit(1);
  }

  if( argc != 3) fatal_error("Invalid parameters in command string !\n\r");

  /*  Decoding Input file  */
  if( (strcmp( argv[1], "-K64" ) == 0) || (strcmp( argv[1], "--key64") == 0) ) {
    inputfile = fopen( argv[2], "r+b" );
    if( !inputfile ) fatal_error("Input file open error!\n\r");
    while( TRUE ) {
      Counter = fread( (void *) buffer_inp, sizeof( char ), READBYTES, inputfile);
      Bytes = code64( DECODE, buffer_inp, Counter, buffer_out, MAXBUFFER );
      if( Bytes == CODE_ERR ) fatal_error("Decode fatal error. Task stopped !\n\r");
      for( i = 0; i < Bytes; i++ ) printf("%c", buffer_out[i]);
      if( Counter < READBYTES ) break;
    }
    fclose( inputfile );
    exit(0);
  }

  /*  Encoding Input file  */
  if( (strcmp( argv[1],"-E64") == 0) || (strcmp( argv[1], "--enc64") == 0) ) {
    inputfile = fopen( argv[2], "r+b" );
    if( !inputfile ) fatal_error("Input file open error! Task stopped !\n\r");
    while( TRUE ) {
      Counter = fread( (void *) buffer_inp, sizeof( char ), READBYTES, inputfile);
      Bytes = code64( ENCODE, buffer_out, MAXBUFFER, buffer_inp, Counter );
      if( Bytes == CODE_ERR ) fatal_error("Encode fatal error, illegal input sequience. Task stopped !\n\r");
      for( i = 0; i < Bytes; i++ ) printf("%c", buffer_out[i]);
      if( Counter < READBYTES ) break;
    }
    fclose( inputfile );
    exit(0);
  }

  fatal_error("Invalid argument[s] in command string !\n\r");
  return(1);
}
