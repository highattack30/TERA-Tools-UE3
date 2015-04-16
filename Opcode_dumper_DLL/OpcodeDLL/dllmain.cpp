#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <direct.h>
#include <string>
#include <fstream>
#include <sstream>

#include "Utils.h"

using namespace std;

char cBuffer[512] = { NULL };
#define Pattern
#define SDK_BASE_DIR				"C:\\Tera"

DWORD	Offset_Name = 0x2C;

#define GObjects_Pattern1			"\xA1\x00\x00\x00\x00\x8B\x00\x00\x8B\x00\x00\x25\x00\x02\x00\x00"
#define GObjects_Mask1  			"x????x??x??xxxxx"
#define GObjects_Offset1			0x1

//\xA1\x00\x00\x00\x00\x8B\x0C\x88\x8B\x01\x81\x49 x????xxxxxxx
#define GNames_Pattern				"\xA1\x00\x00\x00\x00\x8B\x0C\x88\x8B\x01\x81\x49"
#define GNames_Mask					"x????xxxxxxx"
#define GNames_Offset				0x1

DWORD	GObjObjects_offset = NULL;
DWORD	Names_offset = NULL;
string	OpCodeVersion = "3011";
//5D C3 B8 ? ? ? ? 5D C3 B8
//55 8B EC 8B 45 08 0F B7 C0 3D
//\x55\x8B\xEC\x8B\x45\x08\x0F\xB7\xC0\x3D xxxxxxxxxx
DWORD	GetOpCodeNameAddress = 0x191DD80;

template < class T > struct TArray2
{
	T*		Data;
	DWORD	Num;
	DWORD	Max;
};

struct UObject2
{
	UCHAR	Unknown[0x2C];	// unknowed data
	DWORD	NameIndex;				// struct FName
};

struct FNameEntry2
{
	UCHAR	Unknown[0x10];	// unknowed data
	char	Name[1];		// name
};

// Objects and Names arrays
TArray2< UObject2* >*		GObjObjects;
TArray2< FNameEntry2* >*	Names;

ofstream	ofile;
void add_log(char* LOG_FILE, const char *fmt, ...)
{
	ofile.open(LOG_FILE, ios::app);

	va_list va_alist;
	char logbuf[256] = { 0 };

	va_start(va_alist, fmt);
	vsnprintf(logbuf + strlen(logbuf), sizeof(logbuf) - strlen(logbuf), fmt, va_alist);
	va_end(va_alist);

	ofile << logbuf << endl;

	ofile.close();
}

// funcs
PCHAR GetName(UObject2* Object)
{
	DWORD NameIndex = *(PDWORD)((DWORD)Object + Offset_Name);

	//DWORD NameIndexFix = NameIndex * 3;

	if (NameIndex < 0 || NameIndex > Names->Num)
	{
		static char ret[256];
		sprintf_s(ret, "INVALID NAME INDEX : %i > %i", NameIndex, Names->Num);
		return ret;
	}
	else
	{
		//return Names->Data[ NameIndexFix ]->Name;
		return Names->Data[NameIndex]->Name;
	}
}

void ObjectDump()
{
	MODULEINFO miGame = Utils::GetModuleInfo(NULL);

	// log file
	FILE* Log = NULL;
	sprintf_s(cBuffer, "%s\\ObjectDump.txt", SDK_BASE_DIR);
	fopen_s(&Log, cBuffer, "w+");

	sprintf_s(cBuffer, "%s\\logs.txt", SDK_BASE_DIR);
	add_log(cBuffer, "void ObjectDump()\nGObjObjects->Num: %i", GObjObjects->Num);

	fprintf(Log, "(0x%X)\n", (DWORD)miGame.lpBaseOfDll);

	for (DWORD i = 0x0; i < GObjObjects->Num; i++)
	{
		// check if it's a valid object
		if (!GObjObjects->Data[i]) { continue; }


		// log the object
		fprintf(Log, "UObject[%06i] %-50s 0x%X (0x%X) (0x%X)\n", i, GetName(GObjObjects->Data[i]), GObjObjects->Data[i], (DWORD)miGame.lpBaseOfDll - (DWORD)GObjObjects->Data[i], (DWORD)GObjObjects->Data[i] - (DWORD)miGame.lpBaseOfDll);
	}

	// close log
	fclose(Log);
}

void NameDump()
{
	// log file
	FILE* Log = NULL;
	sprintf_s(cBuffer, "%s\\NameDump.txt", SDK_BASE_DIR);
	fopen_s(&Log, cBuffer, "w+");

	sprintf_s(cBuffer, "%s\\logs.txt", SDK_BASE_DIR);
	add_log(cBuffer, "void NameDump()\nNames->Num: %i", Names->Num);

	//for ( DWORD i = 0x0; i < Names->Num; i += 0x3 )
	for (DWORD i = 0x0; i < Names->Num; i++)
	{
		// check if it's a valid object
		if (!Names->Data[i]) { continue; }

		// log the object
		fprintf(Log, "Name[%06i] %s: \t\t0x%X\n", i, Names->Data[i]->Name, Names->Data[i]);
	}

	// close log
	fclose(Log);
}

string GetOpCodeName(DWORD Opcode)
{
	unsigned int pointer = 0;

	_asm
	{
			push Opcode
			call GetOpCodeNameAddress
			add esp, 4
			mov[pointer], eax
	}

	return string((LPCSTR)pointer);
}

DWORD WINAPI Thread(LPVOID nothing)
{
	ofstream fileOut;
	ofstream fileOut2;
	ofstream fileOut3;
	fileOut.open("C:\\Tera\\" + OpCodeVersion + "_OpCodes_Dec.txt", ios::out);
	fileOut2.open("C:\\Tera\\OpCodes.cs", ios::out);
	fileOut3.open("C:\\Tera\\" + OpCodeVersion + "_OpCodes_Hex.txt", ios::out);

	string name = "";

	fileOut2 << "using System;" << endl;
	fileOut2 << "using System.Collections.Generic;" << endl;
	fileOut2 << "using System.Linq;" << endl;
	fileOut2 << "" << endl;
	fileOut2 << "namespace Network" << endl;
	fileOut2 << "{" << endl;
	fileOut2 << "	public enum OpcodeFlags : ushort" << endl;
	fileOut2 << "	{" << endl;

	for (int i = 0; i < 0x10000; i++)
	{
		name = GetOpCodeName(i);
		if (name != "")
		{
			//file dec output
			fileOut << name << " = " << i << endl;

			//file2 hex output
			fileOut2 << "	" << name << " = (ushort)0x" << std::hex << i << endl;

			fileOut3 << name << " = 0x" << std::hex << i << endl; 
		}
	}
	fileOut2 << "	}" << endl;
	fileOut2 << "}" << endl;

	fileOut.close();
	fileOut2.close();
	fileOut3.close();

	return 1;
}

void GetOffsetPositions()
{
	char* Object_Name = "Name";
	char* Object_Outer = "Outer";
	char* Object_Class = "Class";
	char* Object_Object = "Object";
	char* Object_Linker = "Linker";
	char* Object_LinkerIndex = "LinkerIndex";
	char* Object_VfTableObject = "VfTableObject";
	char* Object_ObjectInternalInteger = "ObjectInternalInteger";
	char* Object_HashNext = "HashNext";
	char* Object_HashOuterNext = "HashOuterNext";
	char* Object_StateFrame = "StateFrame";
	char* Object_ObjectArchetype = "ObjectArchetype";
	char* Object_Next = "NextEvaluator";
	char* Object_ObjectFlags = "ObjectFlags";
	char* Object_NetIndex = "NetIndex";

	int Object_Start = 0;
	DWORD Object_ClassPtr = 0;
	DWORD Offset_MaxObjects = 0x4;

	DWORD Offset_1 = 0;
	DWORD Offset_NetIndex = 0;
	DWORD Offset_ObjectFlags = 0;
	DWORD Offset_Next = 0;
	DWORD Offset_ObjectArchetype = 0;
	DWORD Offset_StateFrame = 0;
	DWORD Offset_HashOuterNext = 0;
	DWORD Offset_HashNext = 0;
	DWORD Offset_ObjectInternalInteger = 0;
	DWORD Offset_VfTableObject = 0;
	DWORD Offset_LinkerIndex = 0;
	DWORD Offset_Linker = 0;
	DWORD Offset_Outer = 0;
	DWORD Offset_Class = 0;
	DWORD Offset_Name = 0x2C;
	DWORD Offset_Max = 0x150;
	DWORD Offset_PropertySize = 0;
	DWORD Offset_PropertyOffset = 0;

	sprintf_s(cBuffer, "%s\\Propert_Dump.txt", SDK_BASE_DIR);
	FILE* pPropFile = fopen(cBuffer, "w+");

	sprintf_s(cBuffer, "%s\Propert_Dump2.txt", SDK_BASE_DIR);
	FILE* pPropFile2 = fopen(cBuffer, "w+");

	sprintf_s(cBuffer, "%s\\logs.txt", SDK_BASE_DIR);
	add_log(cBuffer, "void GetOffsetPositions()\nGObjObjects->Num: %i", GObjObjects->Num);

	for (int i = 0; i < GObjObjects->Num; i++)
	{
		DWORD Object = (DWORD)GObjObjects->Data[i];
		if (Object == NULL)
		{
			sprintf_s(cBuffer, "%s\\logs.txt", SDK_BASE_DIR);
			add_log(cBuffer, "Object has returned NULL on step 1");
			continue;
		}

		DWORD Name = *(DWORD*)((DWORD)Object + (DWORD)Offset_Name);
		if (!strcmp(Names->Data[Name]->Name, Object_Name))
		{
			for (DWORD j = Offset_Name; j < Offset_Max; j++)
			{
				DWORD Offset = *(DWORD*)((DWORD)Object + (DWORD)j);
				if (Offset == Offset_Name)
				{
					Offset_PropertyOffset = j;
					Object_Start = i - (Offset_Max / 4);
					goto jmpOne;
				}
			}
		}
	}

jmpOne:
	if (!Offset_PropertyOffset)
	{
		sprintf_s(cBuffer, "%s\\logs.txt", SDK_BASE_DIR);
		add_log(cBuffer, "Offset_PropertyOffset has returned NULL on step 2");
		return;
	}

	for (unsigned long i = Object_Start; i < GObjObjects->Num; i++)
	{
		DWORD Object = (DWORD)GObjObjects->Data[i];
		if (Object == NULL)
		{
			sprintf_s(cBuffer, "%s\\logs.txt", SDK_BASE_DIR);
			add_log(cBuffer, "Object has returned NULL on step 2");
			continue;
		}

		DWORD Name = *(DWORD*)((DWORD)Object + (DWORD)Offset_Name);

		if (!Offset_Outer)
		{
			if (!strcmp(Names->Data[Name]->Name, Object_Outer))
			{
				Offset_Outer = *(DWORD*)((DWORD)Object + (DWORD)Offset_PropertyOffset);
			}
		}

		if (!Offset_Class)
		{
			if (!strcmp(Names->Data[Name]->Name, Object_Class))
			{
				Offset_Class = *(DWORD*)((DWORD)Object + (DWORD)Offset_PropertyOffset);
			}
		}

		if (!Offset_Linker)
		{
			if (!strcmp(Names->Data[Name]->Name, Object_Linker))
			{
				Offset_Linker = *(DWORD*)((DWORD)Object + (DWORD)Offset_PropertyOffset);
			}
		}

		if (!Offset_LinkerIndex)
		{
			if (!strcmp(Names->Data[Name]->Name, Object_LinkerIndex))
			{
				Offset_LinkerIndex = *(DWORD*)((DWORD)Object + (DWORD)Offset_PropertyOffset);
			}
		}

		if (!Offset_VfTableObject)
		{
			if (!strcmp(Names->Data[Name]->Name, Object_VfTableObject))
			{
				Offset_VfTableObject = *(DWORD*)((DWORD)Object + (DWORD)Offset_PropertyOffset);
			}
		}

		if (!Offset_ObjectInternalInteger)
		{
			if (!strcmp(Names->Data[Name]->Name, Object_ObjectInternalInteger))
			{
				Offset_ObjectInternalInteger = *(DWORD*)((DWORD)Object + (DWORD)Offset_PropertyOffset);
			}
		}

		if (!Offset_HashNext)
		{
			if (!strcmp(Names->Data[Name]->Name, Object_HashNext))
			{
				Offset_HashNext = *(DWORD*)((DWORD)Object + (DWORD)Offset_PropertyOffset);
			}
		}

		if (!Offset_HashOuterNext)
		{
			if (!strcmp(Names->Data[Name]->Name, Object_HashOuterNext))
			{
				Offset_HashOuterNext = *(DWORD*)((DWORD)Object + (DWORD)Offset_PropertyOffset);
			}
		}

		if (!Offset_StateFrame)
		{
			if (!strcmp(Names->Data[Name]->Name, Object_StateFrame))
			{
				Offset_StateFrame = *(DWORD*)((DWORD)Object + (DWORD)Offset_PropertyOffset);
			}
		}

		if (!Offset_ObjectArchetype)
		{
			if (!strcmp(Names->Data[Name]->Name, Object_ObjectArchetype))
			{
				Offset_ObjectArchetype = *(DWORD*)((DWORD)Object + (DWORD)Offset_PropertyOffset);
			}
		}

		if (!Offset_ObjectFlags)
		{
			if (!strcmp(Names->Data[Name]->Name, Object_ObjectFlags))
			{
				Offset_ObjectFlags = *(DWORD*)((DWORD)Object + (DWORD)Offset_PropertyOffset);
			}
		}

		if (!Offset_StateFrame)
		{
			if (!strcmp(Names->Data[Name]->Name, Object_StateFrame))
			{
				Offset_StateFrame = *(DWORD*)((DWORD)Object + (DWORD)Offset_PropertyOffset);
			}
		}

		if (!Offset_NetIndex)
		{
			if (!strcmp(Names->Data[Name]->Name, Object_NetIndex))
			{
				Offset_NetIndex = *(DWORD*)((DWORD)Object + (DWORD)Offset_PropertyOffset);
			}
		}

		if (!Offset_Next)
		{
			if (!strcmp(Names->Data[Name]->Name, Object_Next))
			{
				Offset_Next = *(DWORD*)((DWORD)Object + (DWORD)Offset_PropertyOffset);
			}
		}

		Offset_1 = *(DWORD*)((DWORD)Object + (DWORD)Offset_PropertyOffset);
		fprintf(pPropFile2, "Names[%06i]	%-50s 			Offset: 0x%X \n", i, Names->Data[Name]->Name, Offset_1);
	}

	if (!Offset_Outer || !Offset_Class)
	{
		sprintf_s(cBuffer, "%s\\logs.txt", SDK_BASE_DIR);
		add_log(cBuffer, "failed on if (!Offset_Outer || !Offset_Class)");
		return;
	}


	for (unsigned long i = 0; i < GObjObjects->Num; i++)
	{
		DWORD Object = (DWORD)GObjObjects->Data[i];
		if (Object == NULL)
		{
			sprintf_s(cBuffer, "%s\\logs.txt", SDK_BASE_DIR);
			add_log(cBuffer, "Object has returned NULL on step 3");
			continue;
		}

		DWORD ObjectName = *(DWORD*)((DWORD)Object + (DWORD)Offset_Name);
		DWORD Class = *(DWORD*)((DWORD)Object + (DWORD)Offset_Class);
		DWORD ClassOffset = *(DWORD*)((DWORD)Class + (DWORD)Offset_Name);

		if (!strcmp(Names->Data[ClassOffset]->Name, Object_Class) && !strcmp(Names->Data[ObjectName]->Name, Object_Object))
		{
			Object_ClassPtr = Object;
			goto jmpTwo;
		}
	}

jmpTwo:
	if (!Object_ClassPtr)
	{
		sprintf_s(cBuffer, "%s\\logs.txt", SDK_BASE_DIR);
		add_log(cBuffer, "failed on if (!Object_ClassPtr)");
		return;
	}

	for (unsigned long i = 0; i < Offset_MaxObjects; i++)
	{
		DWORD Temp = (Offset_Class + 0x4 + (i * 0x4));
		for (unsigned long j = Offset_Class; j < Offset_Max; j++)
		{
			DWORD Offset = *(DWORD*)((DWORD)Object_ClassPtr + (DWORD)j);
			if (Offset == Temp)
			{
				Offset_PropertySize = j;
				goto jmpThree;
			}
		}
	}

jmpThree:
	if (!Offset_PropertySize)
		return;

	fprintf(pPropFile, "\nclass UObject\n{\npublic:\n");
	fprintf(pPropFile, "\t struct FPointer        VfTableObject;\t			//0x%X (0x04)\n", Offset_VfTableObject);
	fprintf(pPropFile, "\t int                    ObjectInternalInteger;\t	//0x%X (0x04)\n", Offset_ObjectInternalInteger);
	fprintf(pPropFile, "\t struct FQWord          ObjectFlags;\t			//0x%X (0x08)\n", Offset_ObjectFlags);
	fprintf(pPropFile, "\t struct FPointer        HashNext;\t				//0x%X (0x04)\n", Offset_HashNext);
	fprintf(pPropFile, "\t struct FPointer        HashOuterNext;\t			//0x%X (0x04)\n", Offset_HashOuterNext);
	fprintf(pPropFile, "\t int                    NetIndex;\t				//0x%X (0x04)\n", Offset_StateFrame);
	fprintf(pPropFile, "\t class UObject*         Linker;\t					//0x%X (0x04)\n", Offset_Linker);
	fprintf(pPropFile, "\t struct FPointer        LinkerIndex;\t			//0x%X (0x04)\n", Offset_LinkerIndex);
	fprintf(pPropFile, "\t struct FPointer        StateFrame;\t				//0x%X (0x04)\n", Offset_NetIndex);
	fprintf(pPropFile, "\t class UObject*         Outer;\t					//0x%X (0x04)\n", Offset_Outer);
	fprintf(pPropFile, "\t struct FName           Name;\t					//0x%X (0x08)\n", Offset_Name);
	fprintf(pPropFile, "\t class UClass*          Class;\t					//0x%X (0x04)\n", Offset_Class);
	fprintf(pPropFile, "\t class UObject*         ObjectArchetype;\t		//0x%X (0x04)\n", Offset_ObjectArchetype);
	fprintf(pPropFile, "\n}\n");

	fprintf(pPropFile, "\nclass UField : public UObject\n{\npublic:\n");
	fprintf(pPropFile, "\t class UField*		Next;\t						//0x%X (0x04)\n", Offset_Next);
	fprintf(pPropFile, "\n}\n");

	fprintf(pPropFile, "\nUProperty:\n");
	fprintf(pPropFile, "\t- PropertyOffset\t0x%X\n", Offset_PropertyOffset);

	fprintf(pPropFile, "\nUStruct:\n");
	fprintf(pPropFile, "\t- PropertySize\t\t0x%X\n", Offset_PropertySize);

	fclose(pPropFile);
	fclose(pPropFile2);
}

BOOL Init_Core()
{
	sprintf_s(cBuffer, "%s\\logs.txt", SDK_BASE_DIR);
	add_log(cBuffer, "\nstart pattern scan");

	MODULEINFO miGame = Utils::GetModuleInfo(NULL);

	add_log(cBuffer, "GetModuleInfo\nlpBaseOfDll: 0x%X\nSizeOfImage: 0x%X\n", miGame.lpBaseOfDll, miGame.SizeOfImage);


	unsigned long GObjObjects_offset1 = *(unsigned long*)(Utils::FindPattern((DWORD)miGame.lpBaseOfDll, miGame.SizeOfImage, (BYTE*)GObjects_Pattern1, GObjects_Mask1) + GObjects_Offset1);
	GObjObjects = (TArray2< UObject2* >*)GObjObjects_offset1;	// global objects

	unsigned long Names_offset1 = *(unsigned long*)(Utils::FindPattern((DWORD)miGame.lpBaseOfDll, miGame.SizeOfImage, (BYTE*)GNames_Pattern, GNames_Mask) + GNames_Offset);
	Names = (TArray2< FNameEntry2* >*)	Names_offset1;
	
	return true;
}


void onAttach()
{
	Sleep(500);
	sprintf_s(cBuffer, "%s\\logs.txt", SDK_BASE_DIR);
	add_log(cBuffer, "hello and welcome to Domo's scan tool logs tool enabled");

	// mkdir base dir
	_mkdir(SDK_BASE_DIR);

	// mkdir sdk dir
	sprintf_s(cBuffer, "%s", SDK_BASE_DIR);
	_mkdir(cBuffer);

	sprintf_s(cBuffer, "%s\\logs.txt", SDK_BASE_DIR);
	add_log(cBuffer, "successfully created directories");

	if (Init_Core())
	{
		GetOffsetPositions();
		ObjectDump();
		NameDump();

		sprintf_s(cBuffer, "%s\\logs.txt", SDK_BASE_DIR);
		add_log(cBuffer, "tool completed");
	}

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread, NULL, 0, NULL);
}

BOOL WINAPI DllMain(HINSTANCE Hinst, DWORD  Reason, LPVOID nothing)
{
	switch (Reason)
	{

		case DLL_PROCESS_ATTACH:
			MessageBoxA(0, "Injected", "Desu", 0);
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)onAttach, NULL, 0, NULL);
		case DLL_PROCESS_DETACH:
			MessageBoxA(0, "Finished.", "Desu", 0);
	}

	return 1;
}