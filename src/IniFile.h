// ◇
// fds: fdx68 selector
// IniFile: INIファイル読み込み
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__INIFILE_H__)
#define __INIFILE_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <string>
#include <map>

// =====================================================================
// INIファイル読み込み
// =====================================================================

class IniFile
{
public:		// struct, enum

public:		// function
	IniFile() {}
	virtual ~IniFile() {}

	int load(const char* filename);
//	const std::string getString(const char* section, const char* key);
	const std::string getString(const std::string& section, const std::string& key);
//	int getInt(const char* section, const char* key);
	int getInt(const std::string& section, const std::string& key);

	int save(const char* filename);
	void setString(const std::string& section, const std::string& key, const std::string& value);
	void setInt(const std::string& section, const std::string& key, const int value);

private:	// function

public:		// var

private:	// var
	std::map<const std::string,const std::string> mIniFileMap;

};


#endif  // __INIFILE_H__
// =====================================================================
// [EOF]
