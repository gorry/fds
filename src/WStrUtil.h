// ◇
// fds: fdx68 selector
// WStrUtil: wstringユーティリティ
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__WSTRUTIL_H__)
#define __WSTRUTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include <string.h>
#include <limits.h>

#include <string>

#include "FDSCommon.h"
#include "FDSMacro.h"
#include "Log.h"

// =====================================================================
// wstringユーティリティ
// =====================================================================

class WStrUtil
{
public:
	static int copyN(std::wstring& dst, const std::wstring& src, int offset=0, int len=-1);
	static int copyNRight(std::wstring& dst, const std::wstring& src, int offset=0, int len=-1);
	static int copyByWidth(std::wstring& dst, const std::wstring& src, int offset=0, int width=-1);
	static int copyRightByWidth(std::wstring& dst, const std::wstring& src, int offset=0, int width=-1);
	static int widthByWidth(int& retLen, const std::wstring& src, int offset=0, int width=-1);
	static int widthRightByWidth(int& retLen, const std::wstring& src, int offset=0, int width=-1);
	static int widthN(const std::wstring& src, int offset=0, int len=-1);
	static int widthNRight(const std::wstring& src, int offset=0, int len=-1);

	static std::wstring str2wstr(const std::string& str);
	static std::string wstr2str(const std::wstring& wstr);
	static std::string wstr2strN(const std::wstring& wstr, int mbslen);
	static int wstricmp(const std::wstring& wstr1, const std::wstring& wstr2);
	static std::wstring pathSlash2Backslash(const std::wstring& path);
	static std::wstring pathBackslash2Slash(const std::wstring& path);

};


#endif  // __WSTRUTIL_H__
// =====================================================================
// [EOF]
