// ◇
// fds: fdx68 selector
// DirEntry: ディレクトリエントリ
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__DIRENTRY_H__)
#define __DIRENTRY_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>

#include <dirent.h>
#include <curses.h>
#include <sys/stat.h>

#include <algorithm>
#include <string>
#include <vector>

#include "WStrUtil.h"

// =====================================================================
// ディレクトリエントリ
// =====================================================================

class DirEntry
{
public:		// struct, enum
	enum class Type : int {
		Unknown=0,
		ParentDir,
		Dir,
		FdxFile,
		OtherFile,
		Max,
	};

public:		// function
	DirEntry() = delete;
	DirEntry(const std::string& filename, Type t);
	virtual ~DirEntry() {};

	const std::string& filename() const { return mFileName; }
	const std::wstring& wfilename() const { return mWFileName; }
	Type type() const { return mType; }
	bool isParentDir() const { return (mType == Type::ParentDir); }
	bool isNormalDir() const { return (mType == Type::Dir); }
	bool isDir() const { return (mType == Type::Dir)||(mType == Type::ParentDir); }
	bool isFdxFile() const { return (mType == Type::FdxFile); }
	bool isOtherFile() const { return (mType == Type::OtherFile); }
	bool isFile() const { return (mType == Type::OtherFile)||(mType == Type::FdxFile); }
	void setProtect(bool sw) { mProtect = sw; }
	bool isProtect() const { return mProtect; }

private:	// function

public:		// var

private:	// var
	std::string mFileName;
	std::wstring mWFileName;
	Type mType = Type::Unknown;
	bool mProtect = false;

};

// =====================================================================
// ディレクトリ
// =====================================================================

class Directory
{
public:		// struct, enum

public:		// function
	Directory() {}
	Directory(const std::string& path);
	virtual ~Directory() {}

	void setPath(const std::string& path);
	const std::string& getPath() const { return mPath; }
	void getFiles(bool isRoot);
	void sortFiles();
	void clear() { mFiles.clear(); }
	const size_t size() const { return mFiles.size(); }
	const DirEntry& entry(size_t pos) const { return mFiles[pos]; }
	const DirEntry& operator[](size_t pos) const { return entry(pos); }
	void setMaskFdxFile(bool sw);

private:	// function

public:		// var

private:	// var
	std::string mPath;
	std::vector<DirEntry> mFiles;
	bool mMaskFdxFile = false;

};

#endif  // __DIRENTRY_H__
// =====================================================================
// [EOF]
