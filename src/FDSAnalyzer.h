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
#include "FddEmu.h"
#include "FdDump.h"
#include "FdxView.h"
#include "FdxTrkCpy.h"
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
		Data,
		Encode,
		Max,
	};

	enum class AnalyzerMode : int {
		Disk=0,
		Sector,
		Max,
	};

	enum class SectorViewMode : int {
		Data=0,
		Encode,
		Max,
	};

	enum class SectorViewStringEncode : int {
		Ascii=0,
		Sjis,
		Utf16,
		Utf8,
		Max,
	};

	static const int DiskViewRefreshInterval = 500;

public:		// function
	FDSAnalyzer();
	virtual ~FDSAnalyzer() {}

	void setIsWindows(bool b);
	void setFddEmu(FddEmu* pfddemu);
	void start(const std::string& filename, int machineNo);
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
	void cmdDumpTrack();
	static int cmdDumpTrackCallback_(FdDump::Status& st, void* p);
	int cmdDumpTrackCallback(FdDump::Status& st);

	void cmdRestoreTrack();
	static int cmdRestoreTrackCallback_(FdRestore::Status& st, void* p);
	int cmdRestoreTrackCallback(FdRestore::Status& st);

	void cmdMergeTrack();

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
	void trackViewReqReload(void);
	bool trackViewDataIsReady(void);

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
	int sectorViewGetCsrY();
	int sectorViewGetIdx();
	void sectorViewRefresh();
	void sectorViewRedraw();
	void sectorViewRedrawBorder();
	void sectorViewBackDisk();
	void sectorViewSetTrack(int track);
	void sectorViewSetSector(int sector);
	void sectorViewSetLoad(void);
	void sectorViewReqReload(void);
	bool sectorViewDataIsReady(void);
	void sectorViewSetViewMode(SectorViewMode mode);
	void sectorViewSetStringEncode(SectorViewStringEncode enc);

	// help view
	void helpViewCreateWindow();
	void helpViewDestroyWindow();
	void helpViewRefresh();
	void helpViewRedraw();
	void helpViewSetMode(FDSAnalyzer::HelpViewMode md);

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
	std::string mFilename;
	FdxView mFdxView;
	FdDump mFdDump;
	FdRestore mFdRestore;
	FdxView mMergeFdxView;
	FdxTrkCpy mFdxTrkCpy;

	FDSConfig mConfig;
	FddEmu* mpFddEmu;
	bool mIsWindows;

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
	int mTrackViewListLines = 0;

	// sector view
	WINDOW *mwSectorView = nullptr;
	fds::XYWH mSectorViewXYWH = {};
	int mSectorViewInnerW = 0;
	int mSectorViewInnerH = 0;
	int mSectorViewOfsY = 0;
	int mSectorViewCsrY = 0;
	int mSectorViewWindowOfsX = 0;
	int mSectorViewWindowOfsY = 0;
	int mSectorViewTrackNo = 0;
	int mSectorViewSectorNo = 0;
	bool mSectorViewReqLoad = false;
	SectorViewMode mSectorViewMode = SectorViewMode::Data;
	SectorViewStringEncode mSectorViewStringEncode = SectorViewStringEncode::Ascii;
	const int mSectorViewListColumns = 16;
	bool mSectorViewClear = false;
	int mSectorViewListLines = 0;

	// help view
	WINDOW *mwHelpView = nullptr;
	fds::XYWH mHelpViewXYWH = {};
	HelpViewMode mHelpViewMode = HelpViewMode::Disk;
	AnalyzerMode mAnalyzerMode = AnalyzerMode::Disk;

	// dump view
	WINDOW *mwDumpView = nullptr;
	fds::XYWH mDumpViewXYWH = {};
	FdDump::Status mDumpViewStatus;
	int mRetryCount = 0;

	// restore view
	WINDOW *mwRestoreView = nullptr;
	fds::XYWH mRestoreViewXYWH = {};
	FdRestore::Status mRestoreViewStatus;

};


#endif  // __FDSANALYZER_H__
// =====================================================================
// [EOF]
