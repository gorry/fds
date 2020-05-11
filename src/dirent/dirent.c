// ◇
// dirent: Win32用dirent
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "dirent.h"

// =====================================================================
// Win32用dirent
// =====================================================================

// -------------------------------------------------------------
// opendir
// -------------------------------------------------------------
DIR *opendir(const char* name)
{
	wchar_t wname[_MAX_PATH+2];
	int wlen;
	int i;

	if ((name == NULL) || (name[0] == '\0')) {
		return NULL;
	}

	wlen = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, name, -1, wname, _countof(wname));
	if (wlen <= 1) {
		return NULL;
	}
	wlen--;
	for (i=0; i<wlen; i++) {
		if (wname[i] == L'/') {
			wname[i] = L'\\';
		}
	}

	if (wname[wlen - 1] == L'\\') {
		wcscat_s(wname, _countof(wname), L"*");
	}

	DIR* dirp = malloc(sizeof(DIR));
	if (dirp == NULL) {
		return NULL;
	}
	wcscpy_s(dirp->wdir, _countof(dirp->wdir), wname);
	wchar_t* p = PathFindFileNameW(dirp->wdir);
	p[0] = '\0';

	dirp->hnd = FindFirstFileW(wname, &(dirp->wfd));
	if (dirp->hnd == INVALID_HANDLE_VALUE) {
		free(dirp);
		return NULL;
	}
	dirp->count = 0;
	dirp->hasNext = TRUE;

	return dirp;
}

// -------------------------------------------------------------
// readdir
// -------------------------------------------------------------
struct dirent* readdir(DIR* dirp)
{
	int len;

	if (dirp == NULL) {
		return NULL;
	}
	if (!(dirp->hasNext)) {
		return NULL;
	}

	wcscpy_s(dirp->wname, _countof(dirp->wname), dirp->wfd.cFileName);
	len = WideCharToMultiByte(CP_UTF8, 0, dirp->wfd.cFileName, -1, dirp->ent.d_name, _countof(dirp->ent.d_name), NULL, NULL);
	dirp->ent.d_off = dirp->count;
	dirp->ent.d_reclen = sizeof(dirp->ent);
	{
		wchar_t wname[_MAX_PATH*2];
		wcscpy_s(wname, _countof(wname), dirp->wdir);
		wcscat_s(wname, _countof(wname), dirp->wname);
		_wstat(wname, &(dirp->stat));
		dirp->ent.d_ino = dirp->stat.st_ino;
		dirp->ent.d_type = dirp->stat.st_mode;
	}

	dirp->hasNext = FindNextFileW(dirp->hnd, &(dirp->wfd));
	dirp->count++;

	return (&(dirp->ent));
}

// -------------------------------------------------------------
// closedir
// -------------------------------------------------------------
int closedir(DIR* dirp)
{
	if ((dirp == NULL) || (dirp->hnd == INVALID_HANDLE_VALUE)) {
		return -1;
	}

	FindClose(dirp->hnd);
	free(dirp);

	return 0;
}


// =====================================================================
// [EOF]
