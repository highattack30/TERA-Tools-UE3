#pragma once

#ifndef UTILS_H
#define UTILS_H

#pragma comment(lib, "Psapi.lib")
#include <Psapi.h>


#define PI 3.141592f

#define MIN_DIST_WAYPOINTS 0.5

#define IS_POINTER(a) \
	( (a)!=0 && ( (a)&0x3 ) == 0 )

typedef void** PPVOID;
typedef signed short sint16;
typedef signed int sint32;
typedef unsigned char byte;
typedef unsigned __int64 uint64;
typedef signed __int64 sint64;
typedef unsigned int addr;
typedef unsigned int uint32;

class Utils
{
public:
	static DWORD FindPattern( DWORD dwStart, DWORD dwLen, BYTE* pszPatt, char pszMask[] );
	static unsigned int oneAtATimeHash( const char* inpStr );
	static DWORD OffsetToCode( HANDLE hHandle );
	static DWORD GetSizeOfCode( HANDLE hHandle );
	static void AllocateConsole(const char* pTitle);
	static void* PtrFromStaticInterLocked(HMODULE Module,DWORD offset);
	static HMODULE GetModuleHandleInterLocked(const char * hMod);
	//static bool read(addr address, void* str, int size );
	//static bool readFromExeBase( addr address, void* str, int size );
	static MODULEINFO GetModuleInfo ( LPCTSTR lpModuleName );
	static LPBYTE EncryptData(unsigned char* data, unsigned char* key, unsigned int size);
	static LPBYTE DecryptData(unsigned char* data, unsigned char* key, unsigned int size);
	//static void add_log( char* LOG_FILE, const char *fmt, ... );
};

#endif