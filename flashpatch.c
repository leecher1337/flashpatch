/* Adobe Flash timebomb patcher
 *
 * leecher@dose.0wnz.at
 *
 * Simple C-Version without all these .NET dependencies and stuff
 * Based on research by https://github.com/KuromeSan/FlashPatcher/
 *
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <conio.h>
#include "takeown.h"
#ifndef CSIDL_SYSTEM
#define CSIDL_LOCAL_APPDATA 0x001c
#define CSIDL_SYSTEM		0x0025 
#define CSIDL_SYSTEMX86		0x0029 
#endif

static int LocateExes(void);
static int ScanFolder(char *pszPath);
static BYTE *Match (BYTE *lpMem, DWORD dwSize, const BYTE *lpSig, DWORD cbSig, DWORD step);
static BOOL PatchFile(char *pszFile);
static BOOL ShowError(LPSTR pszError);
static BOOL EnablePrivilege (HANDLE hProcess, LPCTSTR lpPrivilegeName);

static int ScanFlashFolder(int nFolder)
{
	char szDir[MAX_PATH+16], *p, *pszDir = szDir+9;
	int nFiles = 0;

	strcpy(szDir, "regsvr32 ");
	if (SHGetSpecialFolderPath(NULL, pszDir, nFolder, FALSE))
	{
		strcat(pszDir, "\\Macromed\\Flash");
		p = pszDir + strlen(pszDir);
		nFiles += ScanFolder(pszDir);
		strcpy(p, "\\flash.ocx");
		if (GetFileAttributes(pszDir) != INVALID_FILE_ATTRIBUTES)
			WinExec(szDir, SW_HIDE);
	}
	return nFiles;
}

static int LocateExes(void)
{
	char szDir[MAX_PATH], *p;
	int nFiles = 0;

	printf ("Scanning...\n");
	nFiles += ScanFlashFolder(CSIDL_SYSTEM);
#ifdef _WIN64
	nFiles += ScanFlashFolder(CSIDL_SYSTEMX86);
#endif
	if (SHGetSpecialFolderPath(NULL, szDir, CSIDL_LOCAL_APPDATA, FALSE))
	{
		p = szDir + strlen(szDir);
		strcpy(p, "\\Google\\Chrome\\User Data\\PepperFlash");
		nFiles += ScanFolder(szDir);
		strcpy(p, "\\Microsoft\\Edge\\User Data\\PepperFlash");
		nFiles += ScanFolder(szDir);
	}
	return nFiles;
}

static int ScanFolder(char *pszPath)
{
	WIN32_FIND_DATA fd;
	HANDLE hSearch;
	size_t cbFileName;
	int nFiles = 0;
	char *p = pszPath + strlen(pszPath);

	*p = '\\'; p++;
	strcpy(p, "*.*");

	if ((hSearch = FindFirstFile(pszPath, &fd)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (_stricmp(fd.cFileName, ".") == 0 ||
					_stricmp(fd.cFileName, "..") == 0)
					continue;
				strcpy(p, fd.cFileName);
				nFiles += ScanFolder(pszPath);
			}
			else
			{
				cbFileName = strlen(fd.cFileName);
				if (cbFileName > 4 &&  (
					_stricmp(&fd.cFileName[cbFileName-4], ".ocx") == 0 ||
					_stricmp(&fd.cFileName[cbFileName-4], ".dll") == 0 ||
					_stricmp(&fd.cFileName[cbFileName-4], ".exe") == 0))
				{
					strcpy(p, fd.cFileName);
					if (PatchFile(pszPath)) nFiles++;
				}
			}
		}
		while(FindNextFile(hSearch, &fd));
		FindClose(hSearch);
	}
	return nFiles;
}

static BYTE *Match (BYTE *lpMem, DWORD dwSize, const BYTE *lpSig, DWORD cbSig, DWORD step)
{
	BYTE *lpOffset, *lpEnd;
	DWORD i;

	for (lpOffset=lpMem, lpEnd=lpMem+dwSize-cbSig; lpOffset<lpEnd; lpOffset+=step)
	{
		for (i=0; i<cbSig; i++)
			if (lpOffset[i]!=lpSig[i]) break;
		if (i==cbSig) return lpOffset;
	}
	return NULL;
}

static void print_ok(char *pszString)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo (hConsole, &csbi);
	SetConsoleTextAttribute (hConsole, (WORD)(FOREGROUND_INTENSITY | FOREGROUND_GREEN));
	printf (pszString);
	SetConsoleTextAttribute (hConsole, csbi.wAttributes);
}

static BOOL PatchFile(char *pszFile)
{
	HANDLE hFile;
	BOOL bRet=FALSE;
	BYTE Timestamp[] = {0x00, 0x00, 0x40, 0x46, 0x3E, 0x6F, 0x77, 0x42};
	BYTE Infinity[]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x7F};
	PBYTE lpSig, lpMem;

	printf ("Checking %s....", pszFile);
	hFile = CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	
	if(hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize;
		HANDLE hMap;

		dwSize = GetFileSize(hFile, NULL);
		if (hMap = CreateFileMapping (hFile, NULL, PAGE_READONLY, 0, 0, NULL))
		{
			if (lpMem = MapViewOfFile (hMap, FILE_MAP_READ, 0, 0, 0))
			{
				if (lpSig = Match(lpMem, dwSize, Timestamp, sizeof(Timestamp), 1))
				{
					char szMsg[32];

					sprintf(szMsg, "Found @%X\n", lpSig-lpMem);
					print_ok(szMsg);
					bRet = TRUE;
				}
				else
					printf("Signature not found\n");
				UnmapViewOfFile (lpMem);
			}
			else
				ShowError("Cannot map view of file");
			CloseHandle (hMap);
		}
		else
			ShowError("Cannot map view of file");
		CloseHandle(hFile);
	}
	else
		ShowError("Cannot open file for reading");
	
	if (bRet)
	{
		bRet = FALSE;
		printf ("Patching...");

		SetFileAttributes(pszFile, GetFileAttributes(pszFile) & ~FILE_ATTRIBUTE_READONLY);
		hFile = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		if (hFile == INVALID_HANDLE_VALUE && GetLastError() != ERROR_SHARING_VIOLATION)
		{
			TakeOwnership(pszFile);
			GrantAllPrivileges(pszFile);
			hFile = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		}
	
		if(hFile != INVALID_HANDLE_VALUE)
		{
			if (SetFilePointer(hFile, (LONG)(lpSig-lpMem), NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER)
			{
				DWORD dwWritten;

				if ((bRet = WriteFile(hFile, Infinity, sizeof(Infinity), &dwWritten, NULL)))
				{
					print_ok("OK\n");
				}
				else
					ShowError("Cannot write patch to file");
			}
			else
				ShowError("Cannot seek to offset");
			CloseHandle(hFile);
		}
		else
			ShowError("Cannot open file for writing");
	}

	return bRet;
}

BOOL ShowModuleError(LPSTR pszModError, LPSTR pszError)
{
	LPTSTR lpMsgBuf;
	DWORD err = GetLastError();
	HANDLE hConsole;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo (hConsole, &csbi);
	SetConsoleTextAttribute (hConsole, (WORD)(FOREGROUND_INTENSITY | FOREGROUND_RED));
    fprintf (stderr, "%s: %s: 0x%08X: %s",
        pszModError, pszError, err, lpMsgBuf);
	SetConsoleTextAttribute (hConsole, csbi.wAttributes);
	LocalFree( lpMsgBuf );
	return FALSE;
}

BOOL ShowError(LPSTR pszError)
{
	return ShowModuleError("Failed patching file", pszError);
}

static BOOL EnablePrivilege (HANDLE hProcess, LPCTSTR lpPrivilegeName)
{
	HANDLE hTok;
	TOKEN_PRIVILEGES tp;
	BOOL bRet = FALSE;

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (OpenProcessToken (hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hTok))
	{
		LookupPrivilegeValue (NULL, lpPrivilegeName, &tp.Privileges[0].Luid);
		bRet = AdjustTokenPrivileges (hTok, FALSE, &tp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
		CloseHandle (hTok);
	}
	return bRet;
}


int main(int argc, char **argv)
{
	int ret;

	printf ("Adobe Flash Timebomb patcher V1.00, leecher@dose.0wnz.at 02/2021\n\n");

	EnablePrivilege (GetCurrentProcess(), SE_BACKUP_NAME);
	EnablePrivilege (GetCurrentProcess(), SE_RESTORE_NAME);
	EnablePrivilege (GetCurrentProcess(), SE_TCB_NAME);

	if (argc < 2) 
	{
		printf ("No path given, scanning default locations:\n");
		ret = LocateExes();
	}
	else
	{
		DWORD dwAttr = GetFileAttributes(argv[1]);

		if (dwAttr == INVALID_FILE_ATTRIBUTES || (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			char szDir[MAX_PATH];

			strcpy(szDir, argv[1]);
			ret = ScanFolder(szDir);
		}
		else
			ret = PatchFile(argv[1]);
	}
	
	printf ("Press any key to exit...");
	getch();
	return ret;
}
