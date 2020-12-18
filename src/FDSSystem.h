// ◇
// fds: fdx68 selector
// FDSSystem: FDSシステム
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__FDSSYSTEM_H__)
#define __FDSSYSTEM_H__

#define FDS_VERSION "20201219a"

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

#include "FDSMacro.h"
#include "DirEntry.h"
#include "DirStack.h"
#include "IniFile.h"
#include "DlgSelect.h"
#include "FddEmu.h"
#include "FdDump.h"
#include "FdRestore.h"
#include "FdxTool.h"
#include "FDXFile.h"
#include "FDSConfig.h"
#include "DlgInput.h"
#include "WStrUtil.h"
#include "Log.h"

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
	enum class ColorPair : short {
		Normal = 1,
		FilerUnknown,
		FilerParentDir,
		FilerDir,
		FilerFdxFile,
		FilerOtherFile,
		FilerUnknownCsr,
		FilerParentDirCsr,
		FilerDirCsr,
		FilerFdxFileCsr,
		FilerOtherFileCsr,
		FilerProtected,
		FilerProtectedCsr,
		Header,
		PathHeader,
		PathRoot,
		PathCurrent,
		SelectHeader,
		SelectItem,
		SelectItemCursor,
		InputHeader,
		InputEdit,
		FddHeaderOff,
		FddHeaderOn,
		FddProtect,
		FddCluster,
		HelpHeader,
		InfoHeader,
		DumpHeader,
		DumpGauge,
		DumpStatusNone,
		DumpStatusFinish,
		DumpStatusUnformat,
		DumpStatusError,
		RestoreHeader,
		RestoreGauge,
		RestoreStatusNone,
		RestoreStatusFinish,
		RestoreStatusUnformat,
		RestoreStatusError,
	};

	enum class HelpViewMode : int {
		ParentDir=0,
		Dir,
		FdxFile,
		OtherFile,
	};

	struct XYWH {
		int X = 0;
		int Y = 0;
		int W = 0;
		int H = 0;
		int x() const { return X; }
		int y() const { return Y; }
		int w() const { return W; }
		int h() const { return H; }
		int l() const { return X; }
		int t() const { return Y; }
		int r() const { return X+W; }
		int b() const { return Y+H; }
	};

	struct ESCKEYMAP {
		const char* keys = nullptr;
		int newkey = 0;
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

	static bool doEscKey(WINDOW* window);
	static bool doEscKeyW(WINDOW* window);
	static const ESCKEYMAP sEscKeyMap[];

private:	// function
	void mainLoop();

	void updateFiles();
	void getFiles();
	void sortFiles();
	void showFiles();

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

	static int cmdDumpDiskCallback_(FdDump::Status& st, void* p);
	int cmdDumpDiskCallback(FdDump::Status& st);

	static int cmdRestoreDiskCallback_(FdRestore::Status& st, void* p);
	int cmdRestoreDiskCallback(FdRestore::Status& st);

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
	FddEmu mFddEmu;
	FdDump mFdDump;
	FdRestore mFdRestore;
	bool mNoRoot;

	FDSConfig mConfig;

	// filer view
	WINDOW *mwFilerView = nullptr;
	XYWH mFilerViewXYWH = {};
	int mFilerViewInnerW = 0;
	int mFilerViewInnerH = 0;
	int mFilerViewOfsY = 0;
	int mFilerViewCsrY = 0;
	int mFilerViewWindowOfsX = 0;
	int mFilerViewWindowOfsY = 0;

	// path view
	WINDOW *mwPathView = nullptr;
	XYWH mPathViewXYWH = {};

	// info view
	WINDOW *mwInfoView = nullptr;
	XYWH mInfoViewXYWH = {};
	std::string mInfoViewFileName;
	FdxHeader mFdxHeader = {};
	int mDiskInfoResult = 0;

	// fdd view
	WINDOW *mwFddView = nullptr;
	XYWH mFddViewXYWH = {};

	// help view
	WINDOW *mwHelpView = nullptr;
	XYWH mHelpViewXYWH = {};
	HelpViewMode mHelpViewMode = HelpViewMode::ParentDir;

	// dump dialog
	int mLastDumpMode = 0;

	// dump view
	WINDOW *mwDumpView = nullptr;
	XYWH mDumpViewXYWH = {};
	FdDump::Status mDumpViewStatus;

	// restore dialog
	int mLastRestoreMode = 0;

	// restore view
	WINDOW *mwRestoreView = nullptr;
	XYWH mRestoreViewXYWH = {};
	FdRestore::Status mRestoreViewStatus;

};


#endif  // __FDSSYSTEM_H__
// =====================================================================
// [EOF]
