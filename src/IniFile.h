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
	int load(const std::string& filename) { return load(filename.c_str()); }
	bool hasKey(const std::string& section, const std::string& key);
	const std::string getString(const std::string& section, const std::string& key, const std::string& defaultparam="");
	int getInt(const std::string& section, const std::string& key, int defaultparam=0);

	int save(const char* filename);
	int save(const std::string& filename) { return save(filename.c_str()); }
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
