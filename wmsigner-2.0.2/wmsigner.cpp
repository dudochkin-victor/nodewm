#include "stdafx.h"
#include "stdio.h"
#include "signer.h"
#include <errno.h>
#include <stdlib.h>
#include "base64.h"
#include "cmdbase.h"

#ifndef ENCODE
#define ENCODE  0
#endif

#ifndef DECODE
#define DECODE  1
#endif

#ifndef CODE_ERR
#define CODE_ERR        0
#endif

const char* WMSignerVersion = "2.0.1";
bool isIgnoreKeyFile = false;
bool isIgnoreIniFile = false;
bool isKWMFileFromCL = false;
char szKeyData[MAXBUF+1] = "";       // Buffer for Signre-s key      
int Key64Flag = FALSE;

int CommandLineParse( const int argc, const char *argv[], char *szLoginCL, char *szPwdCL, 
                      char *szFileNameCL, char *szKeyFileNameCL, char *szKeyData, 
                      char *szStringToSign, int *Key64Flag );

void NormStr( char *str );
int fatal_err( char *err_msg );
int ReadConsoleString( char *str );
int main(int argc, char* argv[]);

/***********************************/

/* .ini file format *************************
   You must use <exe-file name>.ini for
   your ini-file

354413238595
password
/usr/wmsigner/keyfile.kwm
*********************************************/

char* stripCRLF(char* szStrWithCRLF)
{
  if(szStrWithCRLF) {
    if (strlen(szStrWithCRLF)) {
      char *np = szStrWithCRLF;
      if((np = strchr(np, '\n')) !=  NULL) *np = '\0';
      np = szStrWithCRLF;
      if((np = strchr(np, '\r')) !=  NULL) *np = '\0';
    }
  }
  return szStrWithCRLF;
}



bool LoadIniFile(const char *szFName, szptr& szLogin, szptr& szPwd, szptr& szFileName, short &ErrorCode)
{
  char szBufStr[MAXSTR]="";
  bool bRC = false;

 szLogin = "";
 szPwd = "";
 szFileName = "";

 FILE *file = fopen(szFName,"r");
  if (file != NULL)
  {
    if (fgets(szBufStr, MAXSTR, file))
    {
      szLogin = stripCRLF(szBufStr);
      if( strlen( szLogin ) < 1 ) { ErrorCode = -4; return bRC; }
      if (fgets(szBufStr, MAXSTR, file))
      {
        szPwd = stripCRLF(szBufStr);
        if( strlen( szPwd ) < 1 ) { ErrorCode = -5; return bRC; }
        if (fgets(szBufStr, MAXSTR, file))
        {
          szFileName = stripCRLF(szBufStr);
          if( strlen( szFileName ) < 1 ) { ErrorCode = -6; return bRC; }
          bRC = true;
        }
        else
          ErrorCode = -6;   // Keys FileName missing in INI file
      }
      else
        ErrorCode = -5;     // Password missing in INI file
    }
    else
      ErrorCode = -4;       // Login missing in INI file

    fclose(file);
  }
  else
    ErrorCode = 10*errno;

  return bRC;
}

//--------------------------------------------
/*  Parse Command string  */
int CommandLineParse( const int argc, const char *argv[], char *szLoginCL, char *szPwdCL, 
                      char *szFileNameCL, char *szKeyFileNameCL, char *szKeyData, 
                      char *szStringToSign, int *Key64Flag )
{
   int i = 1;
   int j = 0;
   int numparam = 0;
   char KeyBuffer[512];
   size_t Bytes = 0;

   while( i < argc ) {

     j = i + 1;

     if( (strcmp( argv[i], "-h") == 0) || (strcmp( argv[i], "--help") == 0) ){
       printf("wmsigner, Version %s (c) WebMoney Transfer (r), 2007\n\r\n\r", WMSignerVersion );
       printf(" -p   [--password]   : Password for key_file\n\r");
       printf(" -w   [--wmid]       : 123456789012 : WMID (12 digits)\n\r");
       printf(" -s   [--sign]       : string_to_signification : signing specified string\n\r");
       printf(" -i   [--ini-path]   : Correct path to ini_file with ini_file_name *.ini\n\r");
       printf(" -k   [--key-path]   : Correct path to key_file with key_file_name\n\r");
       printf(" -K64 [--key-base64] : Text string in Base64 code, contain the key for wmsigner\n\r");
       printf(" -h   [--help]       : Help (this srceen)\n\r");
       printf(" -v   [--version]    : Version of program\n\r\n\r");
       exit(0);
     }

     if( (strcmp( argv[i], "-v") == 0) || (strcmp( argv[i], "--version") == 0) ){
       printf("wmsigner, Version %s (c) WebMoney Transfer (r), 2007\n\r", WMSignerVersion );
       exit(0);
     }

     if( (strcmp( argv[i], "-p") == 0) || (strcmp( argv[i], "--password") == 0) ) {
       if( j >= argc ) fatal_err("Password not defined!\n\r");
       strncpy( szPwdCL, argv[j], MAXSTR);
       numparam++;
     }

     if( (strcmp( argv[i], "-w") == 0) || (strcmp( argv[i], "--wmid") == 0)) {
       if( j >= argc ) fatal_err("WMID Not defined!\n\r");
       strncpy( szLoginCL, argv[j], MAXSTR);
       numparam++;
     }

     if( (strcmp( argv[i], "-s") == 0) || (strcmp( argv[i], "--sign") == 0)) {
     if( j >= argc ) fatal_err("String to signification not defined!\n\r");
       strncpy( szStringToSign, argv[j], MAXSTR);
       numparam++;
     }

     if( (strcmp( argv[i], "-i") == 0) || (strcmp( argv[i], "--ini-path") == 0)){
       if( j >= argc ) fatal_err("Ini file name (with path) not defined!\n\r");
       strncpy( szFileNameCL, argv[j], MAXSTR);
       numparam++;
     }

     if( (strcmp( argv[i], "-k") == 0) || (strcmp( argv[i], "--key-path") == 0)){
       if( j >= argc ) fatal_err("Key file not defined!\n\r");
       strncpy( szKeyFileNameCL, argv[j], MAXSTR);
       isKWMFileFromCL = true;
       numparam++;
     }

     if( (strcmp( argv[i], "-K64") == 0) || (strcmp( argv[i], "--key-base64") == 0) ){
       if( j >= argc ) fatal_err("KEY_STRING in Base64 code not defined!\n\r");
       if( strlen( argv[j] ) != 220 ) fatal_err("Key string has illegal length!");
       strcpy( szKeyData, argv[j] );
       Bytes = code64( ENCODE, KeyBuffer, 512, szKeyData, 220 );
       if( Bytes != 164 ) fatal_err("Bad key string in parameter!");
       memcpy( szKeyData, KeyBuffer, 164);
       *Key64Flag = TRUE;
       numparam++;
     }

     i+=2;

   }

   if( numparam ) {
     return( TRUE );
   }
   else {
     fatal_err("Illegal command line option found! Use option --help or -h for information.");
   }
   return( FALSE );
}

void NormStr( char *str )
{
  char *s = str;
  size_t i = strlen( s );
  
  if( i == 0 ) return;
  
  if( (s[i]   == '\n')||(s[i]   == '\r')||(s[i] == 0x004) ) s[i]   = 0x00; 
  if( (s[i-1] == '\n')||(s[i-1] == '\r')||(s[i-1] == 0x004) ) s[i-1] = 0x00; 

  i = 0;
    
  while( s[i] ) {
    if( s[i] == '\r' ) strcpy( &s[i], &s[i+1]); 
    if( (s[i] == 0x004) || s[i] == 0x00 ) {
      s[i] = 0;
      return;
    }
    i++;
  }
  return;
}

//--------------------------------------------

int main(int argc, char* argv[])
{
//--------------------------------------------------------------------
  szptr szLogin, szPwd, szFileName, szIn, szSign;
  char szBufforInv[MAXSTR+1] = "";
  char szError[80] = "";
  short siErrCode = 0;
  char szKeyDataENC[MAXBUF+1] = "";    /* Buffer for Signre-s key      */
  char szStringToSign[MAXBUF+1] = "";  /* String for signification     */
  /*-----------------------------------------------------------------*/
  char szLoginCL[MAXSTR+1] = ""; 
  char szPwdCL[MAXSTR+1] = ""; 
  char szFileNameCL[MAXSTR+1] = "";    /*  INI - Fil e                 */
  char szKeyFileNameCL[MAXSTR+1] = ""; /*  KWM - File                  */
  int CmdLineKey = FALSE;              /* ? Command line Key           */
  bool result = FALSE;
  int ErrorCode = 0;
  static char pszOut[MAXBUF+1] = "";
  size_t num_bytes = 0;
  /*-----------------------------------------------------------------*/

  szptr szIniFileFull = "";

#ifdef _WIN32
    char  drive[_MAX_DRIVE] = "";
    char  dir[_MAX_DIR] = "";
    char  fname[_MAX_FNAME] = "";
    char  ext[_MAX_EXT] = "";

    _splitpath((const char *)argv[0], drive, dir, fname, ext );
    szIniFileFull += drive;
    szIniFileFull += dir;
    szIniFileFull += fname;
    szIniFileFull += ".ini";
#else
    // unix sustem
    szIniFileFull = argv[0];
    szIniFileFull += ".ini";
#endif



//---------------------------------------------
 /*  Command line found ? Parsing data */
  if( argc > 1 ) {
    CmdLineKey = CommandLineParse( argc, (const char **)argv, szLoginCL, szPwdCL,
                                   szFileNameCL, szKeyFileNameCL, szKeyData, szStringToSign, &Key64Flag );
  }
  /*  End of Parse command line */

  /*  Replace Key File Name from command Line, if present  */
  if( strlen(szFileNameCL) ) szIniFileFull = szFileNameCL;

  if( ((Key64Flag == TRUE) || (isKWMFileFromCL == true)) && (strlen(szLoginCL) > 1) && (strlen(szPwdCL) >= 1))
          isIgnoreIniFile = true;

  // loading ini-file
  if( isIgnoreIniFile == false )
   if (!LoadIniFile(szIniFileFull, szLogin, szPwd, szFileName, siErrCode))
   {
     sprintf(szError, "Error %d", siErrCode);
     printf(szError);
     exit(2);
     // return 2;
   }

  //  Replace Login and Password from command Line, if present
  if( isKWMFileFromCL == true ) szFileName = szKeyFileNameCL;
  if( strlen(szLoginCL) ) szLogin  = szLoginCL;
  if( strlen(szPwdCL) ) szPwd = szPwdCL;

  // extracting char string
  if( strlen( szStringToSign )) {
    strncpy( szBufforInv, szStringToSign, MAXSTR);
  }
  else {
    if ( ReadConsoleString( szBufforInv ) == 0 ) {
      exit(0);
    }
  }

  NormStr( szBufforInv );

  szIn = szBufforInv;

//----------------------------------------------------
// sigining (new)

  Signer sign(szLogin, szPwd, szFileName);
  sign.isIgnoreKeyFile = isIgnoreKeyFile;
  sign.isIgnoreIniFile = isIgnoreIniFile;
  sign.isKWMFileFromCL = isKWMFileFromCL;
//  szKeyData[MAXBUF+1] = "";       // Buffer for Signre-s key      
  sign.Key64Flag = Key64Flag;


  if( Key64Flag == TRUE ) {
    sign.SetKeyFromCL( TRUE, szKeyData );
  }

  result = sign.Sign(szIn, szSign);
  ErrorCode = sign.ErrorCode();


  if ( result ){
          strncpy( pszOut, szSign, MAXSTR);
      printf("%s", pszOut);
          exit(0);
  }
  else {
    sprintf(pszOut, "Error %d", ErrorCode );
    printf("%s", pszOut);
    return (ErrorCode);
  }
 return 0;
//----------------------------------------------------
}

int ReadConsoleString( char *str )
{
  int i = 0;
  int ch = 0;
  
  while( 1 ) {
    ch = fgetc( stdin );
    if( ch == '\r') continue;
    if( (i == (MAXSTR-1)) || (ch == 0x04) || (ch == 0x00) || (ch == EOF)) { 
      str[i] = 0; 
      return(i);
    }
    str[i++] = (char) ch;
  }
  //return( 0 );
}

int fatal_err( char *err_msg )
{
  printf( "%s", err_msg);
  exit(1);
  return(0);
}
