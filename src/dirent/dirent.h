// ◇
// dirent: Win32用dirent
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__DIRENT_H__)
#define __DIRENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <wchar.h>
#include <locale.h>
#include <windows.h>
#include <sys/stat.h>
#include <shlwapi.h>

#define DT_REG S_IFREG
#define DT_DIR S_IFDIR

// =====================================================================

#if defined(__cplusplus)
extern "C" {
#endif

// =====================================================================
// Win32用dirent
// =====================================================================

struct dirent {
	ino_t d_ino;
	off_t d_off;
	unsigned short d_reclen;
	unsigned int d_type;
	char d_name[_MAX_PATH];
};

typedef struct tagDIR {
	struct dirent ent;
	off_t count;
	BOOL hasNext;
	HANDLE hnd;
	WIN32_FIND_DATAW wfd;

	wchar_t wdir[_MAX_PATH];
	wchar_t wname[_MAX_PATH];
	struct _stat stat;
} DIR;

DIR *opendir(const char* name);
struct dirent* readdir(DIR* dirp);
int closedir(DIR* dirp);

// =====================================================================

#if defined(__cplusplus)
}
#endif

#endif  // __DIRENT_H__
// =====================================================================
// [EOF]
