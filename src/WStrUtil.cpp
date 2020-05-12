// ◇
// fds: fdx68 selector
// WStrUtil: wstringユーティリティ
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "WStrUtil.h"

namespace fds
{
#include "wcwidth/wcwidth.c"
}

// =====================================================================
// wstring文字列ユーティリティ
// =====================================================================

// -------------------------------------------------------------
// src[offset]からlen文字をdstへコピーする
// len<0なら全文字をコピーする
// コピー文字数を返す
// -------------------------------------------------------------
int
WStrUtil::copyN(std::wstring& dst, const std::wstring& src, int offset, int len)
{
	int srclen = src.length();
	if (offset < 0) {
		offset = 0;
	}
	if (offset > srclen) {
		offset = srclen;
	}
	if (len < 0) {
		len = INT_MAX;
	}
	if (len > srclen-offset) {
		len = srclen-offset;
	}
	int w = srclen-offset;
	if (len > w) {
		len = w;
	}

	dst = std::wstring(&src[offset], len);
	return len;
}

// -------------------------------------------------------------
// src[offset]の「右側からlen文字」をdstへコピーする
// len<0なら全文字をコピーする
// コピー文字数を返す
// -------------------------------------------------------------
int
WStrUtil::copyNRight(std::wstring& dst, const std::wstring& src, int offset, int len)
{
	int srclen = src.length();
	if (offset < 0) {
		offset = 0;
	}
	if (offset > srclen) {
		offset = srclen;
	}
	if (len < 0) {
		len = INT_MAX;
	}

	int x = srclen-len;
	if (x < offset) {
		x = offset;
	}
	dst = std::wstring(&src[x]);
	return srclen-x;
}

// -------------------------------------------------------------
// src[offset]から文字幅widthを超えない分の文字列をdstへコピーする
// width<0なら全文字列をコピーする
// コピー文字幅を返す
// -------------------------------------------------------------
int
WStrUtil::copyByWidth(std::wstring& dst, const std::wstring& src, int offset, int width)
{
	int srclen = src.length();
	if (offset < 0) {
		offset = 0;
	}
	if (offset > srclen) {
		offset = srclen;
	}
	if (width < 0) {
		width = INT_MAX;
	}

	int x = offset;
	int w = 0;
	dst.clear();
	while (w < width) {
		wchar_t c = src[x++];
		if (c == L'\0') {
			break;
		}
		int w2 = fds::wcwidth(c);
		if (w+w2 > width) {
			break;
		}
		dst.push_back(c);
		w += w2;
	}
	return w;
}

// -------------------------------------------------------------
// src[offset]の「右側から文字幅widthを超えない分の文字列」をdstへコピーする
// width<0なら全文字列をコピーする
// コピー文字幅を返す
// -------------------------------------------------------------
int
WStrUtil::copyRightByWidth(std::wstring& dst, const std::wstring& src, int offset, int width)
{
	int srclen = src.length();
	if (offset < 0) {
		offset = 0;
	}
	if (offset > srclen) {
		offset = srclen;
	}
	if (width < 0) {
		width = INT_MAX;
	}

	int x = srclen-1;
	int w = 0;
	while (w < width) {
		if (x < offset) {
			break;
		}
		wchar_t c = src[x];
		int w2 = fds::wcwidth(c);
		if (w+w2 > width) {
			break;
		}
		w += w2;
		x--;
	}
	dst = std::wstring(&src[x]);
	return w;
}

// -------------------------------------------------------------
// src[offset]から文字幅widthに収まる文字列の文字幅を求める
// width<0なら全文字列の文字幅を求める
// 文字幅を返す
// -------------------------------------------------------------
int
WStrUtil::widthByWidth(int& retLen, const std::wstring& src, int offset, int width)
{
	int srclen = src.length();
	if (offset < 0) {
		offset = 0;
	}
	if (offset > srclen) {
		offset = srclen;
	}
	if (width < 0) {
		width = INT_MAX;
	}

	int x = offset;
	int w = 0;
	int len = 0;
	while (w < width) {
		wchar_t c = src[x++];
		if (c == L'\0') {
			break;
		}
		int w2 = fds::wcwidth(c);
		if (w+w2 > width) {
			break;
		}
		w += w2;
		len++;
	}
	retLen = len;
	return w;
}

// -------------------------------------------------------------
// src[offset]の「右側から文字幅widthに収まる文字列」の文字幅を求める
// width<0なら全文字列の文字幅を求める
// 文字幅を返す
// -------------------------------------------------------------
int
WStrUtil::widthRightByWidth(int& retLen, const std::wstring& src, int offset, int width)
{
	int srclen = src.length();
	if (offset < 0) {
		offset = 0;
	}
	if (offset > srclen) {
		offset = srclen;
	}
	if (width < 0) {
		width = INT_MAX;
	}

	int x = srclen-1;
	int w = 0;
	while (w < width) {
		if (x < offset) {
			x = offset;
			break;
		}
		wchar_t c = src[x];
		int w2 = fds::wcwidth(c);
		if (w+w2 > width) {
			break;
		}
		w += w2;
		x--;
	}
	retLen = srclen-x;
	return w;
}

// -------------------------------------------------------------
// src[offset]からlen文字の文字列の文字幅を求める
// len<0なら全文字列の文字幅を求める
// 文字幅を返す
// -------------------------------------------------------------
int
WStrUtil::widthN(const std::wstring& src, int offset, int len)
{
	int srclen = src.length();
	if (offset < 0) {
		offset = 0;
	}
	if (offset > srclen) {
		offset = srclen;
	}
	if (len < 0) {
		len = INT_MAX;
	}

	int x = offset;
	int w = 0;
	while (len > 0) {
		wchar_t c = src[x++];
		if (c == L'\0') {
			break;
		}
		int w2 = fds::wcwidth(c);
		w += w2;
		len--;
	}
	return w;
}

// -------------------------------------------------------------
// src[offset]の「右側からlen文字」の文字列の文字幅を求める
// len<0なら全文字列の文字幅を求める
// 文字幅を返す
// -------------------------------------------------------------
int
WStrUtil::widthNRight(const std::wstring& src, int offset, int len)
{
	int srclen = src.length();
	if (offset < 0) {
		offset = 0;
	}
	if (offset > srclen) {
		offset = srclen;
	}
	if (len < 0) {
		len = INT_MAX;
	}

	int x = srclen-1;
	int w = 0;
	while (len > 0) {
		if ((x < 0) || (x < offset)) {
			break;
		}
		wchar_t c = src[x];
		int w2 = fds::wcwidth(c);
		w += w2;
		len--;
		x--;
	}
	return w;
}

// -------------------------------------------------------------
// マルチバイト文字列strをワイド文字列に変換して返す
// 変換はロケールに従って行われる
// -------------------------------------------------------------
std::wstring WStrUtil::str2wstr(const std::string& str)
{
	size_t len = str.length();
	wchar_t* buf = new wchar_t[len+1];
	mbstowcs(buf, str.c_str(), len+1);
	buf[len] = L'\0';
	std::wstring wstr(buf);
	delete[] buf;

	return wstr;
}

// -------------------------------------------------------------
// ワイド文字列wstrをマルチバイト文字列に変換して返す
// 変換はロケールに従って行われる
// -------------------------------------------------------------
std::string WStrUtil::wstr2str(const std::wstring& wstr)
{
	size_t len = wstr.length();
	char* buf = new char[len*6+1];
	wcstombs(buf, wstr.c_str(), len*6);
	buf[len*6] = '\0';
	std::string str(buf);
	delete[] buf;

	return str;
}

// -------------------------------------------------------------
// ワイド文字列の大文字/小文字を区別しない大小比較を行う
// -------------------------------------------------------------
int WStrUtil::wstricmp(const std::wstring& wstr1, const std::wstring& wstr2)
{
	const wchar_t* p1 = &wstr1[0];
	const wchar_t* p2 = &wstr2[0];
	while (!0) {
		wchar_t c1 = towupper(*(p1++));
		wchar_t c2 = towupper(*(p2++));
		if (c1 != c2) {
			return ((c1 > c2) ? 1 : -1);
		}
		if (c1 == L'\0') {
			break;
		}
	}
	return 0;
}

// -------------------------------------------------------------
// ワイド文字列wstrをmaxlenバイト以内のマルチバイト文字列に変換して返す
// 変換はロケールに従って行われる
// maxlenには末尾の'\0'を含む
// -------------------------------------------------------------
std::string WStrUtil::wstr2strN(const std::wstring& wstr, int maxlen)
{
	size_t len = wstr.length();
	char* buf = new char[len*6+1];
	int mbslen = wcstombs(buf, wstr.c_str(), len*6);
	int retlen = 0;
	char* p = buf;
	while (mbslen > 0) {
		int l = mblen(p, mbslen);
		if (l == 0) break;
		if (retlen+l >= maxlen) break;
		p += l;
		retlen += l;
		mbslen -= l;
	}
	p[0] = '\0';
	std::string str(buf);
	delete[] buf;

	return str;
}

// -------------------------------------------------------------
// Path文字列の"/"を"\"に置き換える
// -------------------------------------------------------------
std::wstring WStrUtil::pathSlash2Backslash(const std::wstring& path)
{
	std::wstring newpath;
	int  len = (int)path.length();
	for (int i=0; i<len; i++) {
		wchar_t c = path[i];
		if (c == '/') {
			c = '\\';
		}
		newpath.push_back(c);
	}
	return newpath;
}

// -------------------------------------------------------------
// Path文字列の"\"を"/"に置き換える
// -------------------------------------------------------------
std::wstring WStrUtil::pathBackslash2Slash(const std::wstring& path)
{
	std::wstring newpath;
	int len = (int)path.length();
	for (int i=0; i<len; i++) {
		wchar_t c = path[i];
		if (c == '\\') {
			c = '/';
		}
		newpath.push_back(c);
	}
	return newpath;
}



// =====================================================================
// [EOF]
