#ifndef __OLECF_PARSER_H__
#define __OLECF_PARSER_H__

//experimental
#define WITH_NATIVE_RTF_CONVERT 1

#include <json/json.h>
#include <sstream>
#include <iostream>

#include <string>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define BUFLEN 8192

#include "libolecf.h"

#if defined(_WIN32)
#include <windows.h>
#include <richedit.h>
#include <commctrl.h>
#include <tchar.h>
#else
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#if WITH_NATIVE_RTF_CONVERT
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#endif
#endif

#ifdef __APPLE__
#include "librtf.h"
#else
#include "librtf (windows).h"
#endif
#include "RtfReader.h"

#include <tidy.h>
#include <tidybuffio.h>

#ifdef _WIN32
#define _unlink DeleteFile
#define _libolecf_file_open libolecf_file_open_wide
#else
#define _unlink unlink
#define _libolecf_file_open libolecf_file_open
#endif

#ifdef __APPLE__
#define _fopen fopen
#define _fseek fseek
#define _ftell ftell
#define _rb "rb"
#define _wb "wb"
#else
#define _fopen _wfopen
#define _fseek _fseeki64
#define _ftell _ftelli64
#define _rb L"rb"
#define _wb L"wb"
#endif

#ifdef __APPLE__
#define OPTARG_T char*
#include <getopt.h>
#else
#ifndef _WINGETOPT_H_
#define _WINGETOPT_H_
#define OPTARG_T wchar_t*
#define main wmain
#include <shellapi.h> 
#define NULL    0
#define EOF    (-1)
#define ERR(s, c)    if(opterr){\
char errbuf[2];\
errbuf[0] = c; errbuf[1] = '\n';\
fputws(argv[0], stderr);\
fputws(s, stderr);\
fputwc(c, stderr);}
#ifdef __cplusplus
extern "C" {
#endif
    extern int opterr;
    extern int optind;
    extern int optopt;
    extern OPTARG_T optarg;
    extern int getopt(int argc, OPTARG_T *argv, OPTARG_T opts);
#ifdef __cplusplus
}
#endif
#endif  /* _WINGETOPT_H_ */
#endif

#endif  /* __OLECF_PARSER_H__ */
