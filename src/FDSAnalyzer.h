// ◇
// fds: fdx68 selector
// FDSAnalyzer: FDSアナライザ
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__FDSANALYZER_H__)
#define __FDSANALYZER_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <string.h>

#include <curses.h>

#include <string>
#include <vector>
#include <algorithm>
#include <array>

#include "FDSCommon.h"
#include "FDSMacro.h"
#include "IniFile.h"
#include "DlgSelect.h"
#include "FdDump.h"
#include "FdxView.h"
#include "FDXFile.h"
#include "FDSConfig.h"
#include "DlgInput.h"
#include "WStrUtil.h"
#include "Log.h"

// =====================================================================
// FDSアナライザ
// =====================================================================

class FDSAnalyzer
{
public:		// struct, enum
	enum class HelpViewMode : int {
		Disk=0,
		Track,
		Sector,
	};

	static const int DiskViewRefreshInterval = 500;

public:		// function
	FDSAnalyzer() {}
	virtual ~FDSAnalyzer() {}

	void start(const std::string& filename);
	void end();

private:	// function
	void mainLoop();

	void initView();
	void destroyView();
	void drawHeader();

	int loadIniFile();

	void setViewLayout();

	void refreshAllView();

	// cmd
#if 0
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
#endif

	// disk view
	void diskViewCreateWindow();
	void diskViewDestroyWindow();
	void diskViewSetCsr(int x, int y);
	void diskViewSetIdx(int idx);
	void diskViewUpCursor();
	void diskViewDownCursor();
	void diskViewLeftCursor();
	void diskViewRightCursor();
	void diskViewPageUpCursor();
	void diskViewPageDownCursor();
	void diskViewPageTopCursor();
	void diskViewPageBottomCursor();
	int diskViewGetIdx();
	void diskViewRefresh();
	void diskViewRedraw();
	void diskViewRedrawBorder();
	void diskViewSelectEntry();
	void diskViewShowTrack();

	// track view
	void trackViewCreateWindow();
	void trackViewDestroyWindow();
	void trackViewSetViewOfsY(int idx);
	int trackViewGetViewOfsY();
	void trackViewSetCsrY(int y);
	void trackViewSetIdx(int idx);
	void trackViewUpCursor();
	void trackViewDownCursor();
	void trackViewPageUpCursor();
	void trackViewPageDownCursor();
	void trackViewPageTopCursor();
	void trackViewPageBottomCursor();
	int trackViewGetCsrY();
	int trackViewGetIdx();
	void trackViewRefresh();
	void trackViewRedraw();
	void trackViewRedrawBorder();
	void trackViewSelectEntry();
	void trackViewBackDisk();
	void trackViewShowSector();
	void trackViewSetCylinder(int cylinder);
	void trackViewSetHead(int head);
	void trackViewSetLoad(void);

	// sector view
	void sectorViewCreateWindow();
	void sectorViewDestroyWindow();
	void sectorViewSetViewOfsY(int idx);
	int sectorViewGetViewOfsY();
	void sectorViewSetCsrY(int y);
	void sectorViewSetIdx(int idx);
	void sectorViewUpCursor();
	void sectorViewDownCursor();
	void sectorViewPageUpCursor();
	void sectorViewPageDownCursor();
	void sectorViewPageTopCursor();
	void sectorViewPageBottomCursor();
	void sectorViewRefresh();
	void sectorViewRedraw();
	void sectorViewBackTrack();
	void sectorViewSetSector(int sector);

	// help view
	void helpViewCreateWindow();
	void helpViewDestroyWindow();
	void helpViewRefresh();
	void helpViewRedraw();
	void helpViewSetMode(FDSAnalyzer::HelpViewMode md);

public:		// var

private:	// var
	std::string mFilename;
	FdxView mFdxView;
	FdDump mFdDump;

	FDSConfig mConfig;

	// disk view
	WINDOW *mwDiskView = nullptr;
	fds::XYWH mDiskViewXYWH = {};
	int mDiskViewInnerW = 0;
	int mDiskViewInnerH = 0;
	int mDiskViewCsr = 0;
	int mDiskViewWindowOfsX = 0;
	int mDiskViewWindowOfsY = 0;

	// track view
	WINDOW *mwTrackView = nullptr;
	fds::XYWH mTrackViewXYWH = {};
	int mTrackViewInnerW = 0;
	int mTrackViewInnerH = 0;
	int mTrackViewOfsY = 0;
	int mTrackViewCsrY = 0;
	int mTrackViewWindowOfsX = 0;
	int mTrackViewWindowOfsY = 0;
	int mTrackViewTrackNo = 0;
	int mTrackViewCylinderNo = 0;
	int mTrackViewHeadNo = 0;
	bool mTrackViewReqLoad = false;
	bool mTrackViewClear = false;

	// sector view
	WINDOW *mwSectorView = nullptr;
	fds::XYWH mSectorViewXYWH = {};
	int mSectorViewInnerW = 0;
	int mSectorViewInnerH = 0;
	int mSectorViewOfsY = 0;
	int mSectorViewCsrY = 0;
	int mSectorViewWindowOfsX = 0;
	int mSectorViewWindowOfsY = 0;

	// help view
	WINDOW *mwHelpView = nullptr;
	fds::XYWH mHelpViewXYWH = {};
	HelpViewMode mHelpViewMode = HelpViewMode::Disk;

};


#endif  // __FDSANALYZER_H__
// =====================================================================
// [EOF]
