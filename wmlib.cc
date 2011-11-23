/**
 * FAST nodejs(http://github.com/ry/node/) library for making hashes
 *
 * @package wmlib
 * @link http://github.com/brainfucker/hashlib
 * @autor Oleg Illarionov <oleg@emby.ru>
 * @version 1.0
 */

#include <iostream>
#include <stdio.h>

#include <v8.h>
#include <ev.h>
#include <eio.h>
#include <fcntl.h>

#include "stdafx.h"
#include "stdio.h"
#include "signer.h"
#include <errno.h>
#include <stdlib.h>
#include "base64.h"
#include "cmdbase.h"

using namespace v8;

void NormStr(char *str);

static char pszOut[MAXBUF + 1] = "";

Handle<Value> sign(const Arguments& args) {
	HandleScope scope;
	String::Utf8Value uLogin(args[0]->ToString());
	String::Utf8Value uPwd(args[1]->ToString());
	String::Utf8Value uKeyData(args[2]->ToString());
	String::Utf8Value uSignData(args[3]->ToString());

	szptr szLogin, szPwd, szFileName, szIn, szSign;
	char szBufforInv[MAXSTR + 1] = "";
	char szKeyData[MAXBUF + 1] = ""; // Buffer for Signre-s key
	int ErrorCode = 0;
	bool result = FALSE;

	char szLoginCL[MAXSTR+1] = "";
	char szPwdCL[MAXSTR+1] = "";
	strncpy( szLoginCL, (char*) *uLogin, MAXSTR);
	strncpy( szPwdCL, (char*) *uPwd, MAXSTR);
	szLogin = szLoginCL;
	szPwd = szPwdCL;

	size_t Bytes = 0;
	char KeyBuffer[512];

	if (strlen((char*) *uKeyData) != 220)
		return scope.Close(ThrowException(Exception::Error(String::New((char*)"Key string has illegal length!"))));
	strcpy(szKeyData, (char*) *uKeyData);
	Bytes = code64(ENCODE, KeyBuffer, 512, szKeyData, 220);
	if (Bytes != 164)
		return scope.Close(ThrowException(Exception::Error(String::New((char*)"Bad key string in parameter!"))));
	memcpy(szKeyData, KeyBuffer, 164);

	if( strlen( (char*) *uSignData )) {
		strncpy( szBufforInv, (char*) *uSignData, MAXSTR);
	}
	NormStr(szBufforInv);
	szIn = szBufforInv;

	//----------------------------------------------------
	// sigining (new)

	Signer sign(szLogin, szPwd, szFileName);
	sign.isIgnoreKeyFile = false;
	sign.isIgnoreIniFile = true;
	sign.isKWMFileFromCL = false;
	sign.Key64Flag = true;

	sign.SetKeyFromCL(TRUE, szKeyData);

	result = sign.Sign(szIn, szSign);
	ErrorCode = sign.ErrorCode();

	if (result) {
		strncpy(pszOut, szSign, MAXSTR);
		//printf("%s", pszOut);
		return scope.Close(String::New((char*) pszOut));
	} else {
		sprintf(pszOut, "WMSigner Error: %d\n", ErrorCode);
		//printf("%s", pszOut);
		return scope.Close(ThrowException(Exception::Error(String::New(pszOut))));
	}
}

//Handle<Value> md6(const Arguments& args) {
	//  HandleScope scope;
	//
	//  String::Utf8Value data(args[0]->ToString());
	//
	//  int len(32);
	//  if (!args[1]->IsUndefined()) {
	//    len=args[1]->ToInteger()->Value();
	//  }
	//  unsigned char digest[len];
	//  unsigned char hexdigest[len];
	//  md6_hash(len*8, (unsigned char*) *data, data.length(), digest);
	//
	//  int half_len=len/2;
	//  if (len%2!=0) half_len++;
	//
	//  make_digest_ex(hexdigest, digest, half_len);
	//
	//  return scope.Close(String::New((char*)hexdigest,len));
//}

//int read_cb(eio_req *req) {
	//  file_data *fd=(file_data *)req->data;
	//  unsigned char *buf = (unsigned char *)EIO_BUF (req);
	//  int bytes=(int)EIO_RESULT(req);
	//  MD5Update (&fd->mdContext, buf, bytes);
	//  if (bytes==10240) {
	//  	// Read next block
	//  	fd->byte+=bytes;
	//  	eio_read(fd->fd, 0, 10240, fd->byte, EIO_PRI_DEFAULT, read_cb, static_cast<void*>(fd));
	//  } else {
	//  	// Final
	//  	unsigned char digest[16];
	//  	unsigned char hexdigest[32];
	//  	MD5Final(digest, &fd->mdContext);
	//  	make_digest_ex(hexdigest, digest, 16);
	//
	//  	Persistent<Object> *data = reinterpret_cast<Persistent<Object>*>(fd->environment);
	//
	//    v8::Handle<v8::Function> callback = v8::Handle<v8::Function>::Cast((*data)->Get(String::New("callback")));
	//    Handle<Object> recv = Handle<Object>::Cast((*data)->Get(String::New("recv")));
	//    v8::Handle<v8::Value> outArgs[] = {String::New((char *)hexdigest,32)};
	//    callback->Call(recv, 1, outArgs);
	//    data->Dispose();
	//    eio_close(fd->fd, 0, 0, (void*)"close");
	//    ev_unref(EV_DEFAULT_UC);
	//  }
	//
	//  return 0;
//}

//Handle<Value> get_md5_file_async(char * path, void* data) {
	//  eio_open (path, O_RDONLY, 0777, 0, open_cb, data);
	//  return v8::Boolean::New(true);
//}

//Handle<Value> md5_file(const Arguments& args) {
	//  HandleScope scope;
	//  struct stat stFileInfo;
	//  String::Utf8Value path(args[0]->ToString());
	//  char* cpath=*path;
	//  int intStat = stat(cpath,&stFileInfo);
	//  if (intStat == 0) {
	//    if (args[1]->IsFunction()) {
	//	  v8::Local<v8::Object> arguments = v8::Object::New();
	//	  arguments->Set(String::New("path"),args[0]->ToString());
	//	  arguments->Set(String::New("callback"),args[1]);
	//	  arguments->Set(String::New("recv"),args.This());
	//	  Persistent<Object> *data = new Persistent<Object>();
	//	  *data = Persistent<Object>::New(arguments);
	//
	//    file_data *fd=new file_data;
	//	  fd->byte = 0;
	//	  fd->environment = data;
	//
	//	  MD5Init(&fd->mdContext);
	//	  ev_ref(EV_DEFAULT_UC);
	//    	return scope.Close(get_md5_file_async(cpath,static_cast<void*>(fd)));
	//    } else {
	//    	return scope.Close(get_md5_file(cpath));
	//    }
	//  } else {
	//    std::string s="Cannot read ";
	//    s+=cpath;
	//    return scope.Close(ThrowException(Exception::Error(String::New(s.c_str()))));
	//  }
//}

extern "C" void init(Handle<Object> target) {
	HandleScope scope;
	target->Set(String::New("sign"), FunctionTemplate::New(sign)->GetFunction());
}
