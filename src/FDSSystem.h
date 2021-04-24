// ◇
// fds: fdx68 selector
// FDSSystem: FDSシステム
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__FDSSYSTEM_H__)
#define __FDSSYSTEM_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <curses.h>

#include <string>
#include <vector>
#include <algorithm>
#include <array>

#include "FDSCommon.h"
#include "FDSMacro.h"
#include "DirEntry.h"
#include "DirStack.h"
#include "IniFile.h"
#include "DlgSelect.h"
#include "FddEmu.h"
#include "FdDump.h"
#include "FdRestore.h"
#include "FdxTool.h"
#include "FdxView.h"
#include "FdxConv.h"
#include "FDXFile.h"
#include "FDSConfig.h"
#include "DlgInput.h"
#include "WStrUtil.h"
#include "Log.h"
#include "FDSAnalyzer.h"

#if defined(FDS_WINDOWS)
#include <direct.h>
#include <io.h>
#endif  // defined(FDS_WINDOWS)

// =====================================================================
// FDSシステム
// =====================================================================

class FDSSystem
{
public:		// struct, enum
	enum class HelpViewMode : int {
		ParentDir=0,
		Dir,
		FdxFile,
		OtherImageFile,
		OtherFile,
	};

	static const int FddViewRefreshInterval = 500;

public:		// function
	FDSSystem() {}
	virtual ~FDSSystem() {}

	void setRootDir(const std::string& rootDir);
	void setCurDir(const std::string& curDir);
	void setNoRoot(bool sw);
	void start();
	void end();

private:	// function
	void mainLoop();

	void initView();
	void destroyView();
	void drawHeader();

	int loadIniFile();

	void setViewLayout();

	void refreshAllView();

	const std::string& getDriveName(int id);

	// cmd
	void cmdRename();
	void cmdAutoSet();
	void cmdEjectAllDrive();
	void cmdEjectDrive();
	void cmdSetDrive(int id);
	void cmdCreateDisk();
	void cmdDupDisk();
	void cmdMakeDirectory();
	void cmdEditName();
	void cmdEditProtect();
	void cmdProtectDrive();
	void cmdProtectDisk();
	void cmdDelete();
	void cmdShell();
	void cmdDumpDisk();
	void cmdRestoreDisk();
	void cmdAnalyzeDisk();

	static int cmdDumpDiskCallback_(FdDump::Status& st, void* p);
	int cmdDumpDiskCallback(FdDump::Status& st);

	static int cmdRestoreDiskCallback_(FdRestore::Status& st, void* p);
	int cmdRestoreDiskCallback(FdRestore::Status& st);

	void cmdConvertDisk();
	void cmdConvertDiskFromFdx();
	void cmdConvertDiskToFdx();
	int cmdConvertDiskFdxToFdx();

	// filer view
	void filerViewCreateWindow();
	void filerViewDestroyWindow();
	void filerViewSetViewOfsY(int idx);
	int filerViewGetViewOfsY();
	void filerViewSetCsrY(int y);
	void filerViewSetIdx(int idx);
	void filerViewUpCursor();
	void filerViewDownCursor();
	void filerViewPageUpCursor();
	void filerViewPageDownCursor();
	void filerViewPageTopCursor();
	void filerViewPageBottomCursor();
	int filerViewGetCsrY();
	int filerViewGetIdx();
	void filerViewRefresh();
	void filerViewRedraw();
	void filerViewSelectEntry();
	void filerViewBackDir();
	void filerViewRootDir();
	void filerViewShowFileInfo();
	int filerViewFindEntry(const std::string& filename);

	// path view
	void pathViewCreateWindow();
	void pathViewDestroyWindow();
	void pathViewRefresh();
	void pathViewRedraw();

	// info view
	void infoViewCreateWindow();
	void infoViewDestroyWindow();
	void infoViewRefresh();
	void infoViewRedraw();
	void infoViewSetFile(const std::string& filename);
	int infoViewLoadFile();
	int infoViewSaveFile();
	int infoViewGetResult() const { return mDiskInfoResult; }
	FdxHeader& infoViewGetFdxHeader() { return mFdxHeader; }

	// fdd view
	void fddViewCreateWindow();
	void fddViewDestroyWindow();
	void fddViewRefresh();
	void fddViewRedraw();

	// help view
	void helpViewCreateWindow();
	void helpViewDestroyWindow();
	void helpViewRefresh();
	void helpViewRedraw();
	void helpViewSetMode(FDSSystem::HelpViewMode md);

	// dump view
	void dumpViewCreateWindow();
	void dumpViewDestroyWindow();
	void dumpViewRefresh();
	void dumpViewRedraw();
	void dumpViewUpdate(FdDump::Status& st);

	// restore view
	void restoreViewCreateWindow();
	void restoreViewDestroyWindow();
	void restoreViewRefresh();
	void restoreViewRedraw();
	void restoreViewUpdate(FdRestore::Status& st);

public:		// var

private:	// var
	std::string mRootDir;
	std::string mCurDir;
	Directory mFiles;
	DirStack mDirStack;
	IniFile mSavFile;
	FdxTool mFdxTool;
	FdxView mFdxView;
	FddEmu mFddEmu;
	FdDump mFdDump;
	FdRestore mFdRestore;
	FdxConv mFdxConv;
	bool mNoRoot;

	FDSConfig mConfig;

	bool mIsWindows;

	// filer view
	WINDOW *mwFilerView = nullptr;
	fds::XYWH mFilerViewXYWH = {};
	int mFilerViewInnerW = 0;
	int mFilerViewInnerH = 0;
	int mFilerViewOfsY = 0;
	int mFilerViewCsrY = 0;
	int mFilerViewWindowOfsX = 0;
	int mFilerViewWindowOfsY = 0;

	// path view
	WINDOW *mwPathView = nullptr;
	fds::XYWH mPathViewXYWH = {};

	// info view
	WINDOW *mwInfoView = nullptr;
	fds::XYWH mInfoViewXYWH = {};
	std::string mInfoViewFileName;
	FdxHeader mFdxHeader = {};
	int mDiskInfoResult = 0;

	// fdd view
	WINDOW *mwFddView = nullptr;
	fds::XYWH mFddViewXYWH = {};

	// help view
	WINDOW *mwHelpView = nullptr;
	fds::XYWH mHelpViewXYWH = {};
	HelpViewMode mHelpViewMode = HelpViewMode::ParentDir;

	// dump view
	WINDOW *mwDumpView = nullptr;
	fds::XYWH mDumpViewXYWH = {};
	FdDump::Status mDumpViewStatus;

	// restore view
	WINDOW *mwRestoreView = nullptr;
	fds::XYWH mRestoreViewXYWH = {};
	FdRestore::Status mRestoreViewStatus;

};


#endif  // __FDSSYSTEM_H__
// =====================================================================
// [EOF]
