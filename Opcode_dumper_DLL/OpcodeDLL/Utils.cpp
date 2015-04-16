#include "stdafx.h"
#include <windows.h>
#include "Utils.h"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <time.h>
#include <winbase.h>
#include "wininet.h"

using namespace std;

void Utils::AllocateConsole(const char* pTitle)
{
	// Allocate Console Window
	AllocConsole() ;
	AttachConsole( GetCurrentProcessId() );
	freopen( "CON", "w", stdout ) ;
	SetConsoleTitleA( pTitle );

	// Resize console (max length)
	COORD cordinates = {80, 32766};
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleScreenBufferSize(handle, cordinates);
}

DWORD Utils::FindPattern( DWORD dwStart, DWORD dwLen, BYTE* pszPatt, char pszMask[] )
{
	unsigned int i = NULL;
	int iLen = strlen( pszMask ) - 1;

	for( DWORD dwRet = dwStart; dwRet < dwStart + dwLen; dwRet++ )
	{
		if( *(BYTE*)dwRet == pszPatt[i] || pszMask[i] == '?' )
		{
			if( pszMask[i+1] == '\0' )
				return( dwRet - iLen );
			i++;
		} 
		else 
			i = NULL;
	}
	return NULL;
}

DWORD Utils::GetSizeOfCode( HANDLE hHandle )
{
	HMODULE hModule = (HMODULE)hHandle;

	if ( !hModule )
		return NULL;

	PIMAGE_DOS_HEADER pDosHeader = PIMAGE_DOS_HEADER( hModule );

	if( !pDosHeader )
		return NULL;

	PIMAGE_NT_HEADERS pNTHeader = PIMAGE_NT_HEADERS( (LONG)hModule + pDosHeader->e_lfanew );

	if( !pNTHeader )
		return NULL;

	PIMAGE_OPTIONAL_HEADER pOptionalHeader = &pNTHeader->OptionalHeader;

	if( !pOptionalHeader )
		return NULL;

	return pOptionalHeader->SizeOfCode;
}

DWORD Utils::OffsetToCode( HANDLE hHandle )
{
	HMODULE hModule = (HMODULE)hHandle;

	if ( !hModule )
		return NULL;

	PIMAGE_DOS_HEADER pDosHeader = PIMAGE_DOS_HEADER( hModule );

	if( !pDosHeader )
		return NULL;

	PIMAGE_NT_HEADERS pNTHeader = PIMAGE_NT_HEADERS( (LONG)hModule + pDosHeader->e_lfanew );

	if( !pNTHeader )
		return NULL;

	PIMAGE_OPTIONAL_HEADER pOptionalHeader = &pNTHeader->OptionalHeader;

	if( !pOptionalHeader )
		return NULL;

	return pOptionalHeader->BaseOfCode;
}

void* Utils::PtrFromStaticInterLocked(HMODULE Module,DWORD offset)
{
	void * ptr = (void*)((DWORD)Module+offset);
	while (!ptr)
	{
		ptr = (void*)((DWORD)Module+offset);
		Sleep(20);
	}
	return ptr;
}

HMODULE Utils::GetModuleHandleInterLocked(const char * hMod)
{
	HMODULE hModule = GetModuleHandleA(hMod);
	while (!hModule)
	{
		hModule = GetModuleHandleA(hMod);
		Sleep(20);
	}
	return hModule;
}

//bool Utils::read(addr address, void* str, int size )
//{
//	return ReadProcessMemory( Entry::g_hMainModule, (LPCVOID)address, str, size, NULL) != 0;
//}
//
//bool Utils::readFromExeBase( addr address, void* str, int size )
//{
//	return read( (addr)Entry::g_hMainModule + address, str, size );
//}

MODULEINFO Utils::GetModuleInfo ( LPCTSTR lpModuleName )
{
	MODULEINFO miInfos = { NULL };

	HMODULE hmModule = GetModuleHandle ( lpModuleName );

	if ( hmModule )
	{
		GetModuleInformation ( GetCurrentProcess(), hmModule, &miInfos, sizeof ( MODULEINFO ) );
	}

	return miInfos;
}

//ofstream	ofile;
//void Utils::add_log( char* LOG_FILE, const char *fmt, ... )
//{
//	ofile.open( LOG_FILE, ios::app );
//
//	va_list va_alist;
//	char logbuf[256] = {0};
//
//	va_start( va_alist, fmt );
//	vsnprintf( logbuf + strlen(logbuf), sizeof(logbuf) - strlen(logbuf), fmt, va_alist );
//	va_end( va_alist );
//
//	ofile << logbuf << endl;
//
//	ofile.close();
//}

LPBYTE Utils::DecryptData(unsigned char* data, unsigned char* key, unsigned int size) 
{
	unsigned int A = 0, B = 0, C = 0, D = 0;

	for (unsigned int i = 0, keycounter = 0; i < size; i++, keycounter++) {
		A = data[i];
		B = C & 0xFF;
		C = A;
		A ^= key[i % 0xF];
		A -= i;
		A ^= B;
		data[i] = A;
	}
	return data;
}

LPBYTE Utils::EncryptData(unsigned char* data, unsigned char* key, unsigned int size) 
{
	unsigned int A = 0, B = 0;

	for (unsigned int i = 0, keycounter = 0; i < size; i++, keycounter++) {
		A = data[i];
		B &= 0xFF;
		B ^= A;
		B += i;
		B &= 0xFF;
		B ^= key[i % 0xF];
		data[i] = B;
	}
	return data;
}