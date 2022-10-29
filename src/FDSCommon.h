// ◇
// fds: fdx68 selector
// FDSCommon: FDS共通
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__FDSCOMMON_H__)
#define __FDSCOMMON_H__

#define FDS_VERSION "20221029a"

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

#include "FDSMacro.h"
#include "WStrUtil.h"
#include "Log.h"
#include "FDXFile.h"

namespace fds {

// =====================================================================
// 構造体定義
// =====================================================================

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

enum class ColorPair : short {
	Normal = 1,
	FilerUnknown,
	FilerParentDir,
	FilerDir,
	FilerFdxFile,
	FilerOtherImageFile,
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
	DumpStatusCsr,
	RestoreHeader,
	RestoreGauge,
	RestoreStatusNone,
	RestoreStatusFinish,
	RestoreStatusUnformat,
	RestoreStatusError,
	TrackNormalSec,
	TrackErrorSec,
	TrackF5F6F7Sec,
	TrackNormalSecCsr,
	TrackErrorSecCsr,
	TrackF5F6F7SecCsr,
	TrackAbnormalItem,
	TrackStatus,
	TrackStatusFM,
	TrackGauge,
	SectorNormal,
	SectorNormalCsr,
	SectorBit0,
	SectorBit3,
	SectorBit6,
	SectorBit7,
	SectorBit8,
	SectorBitB,
	SectorGauge,
};


// =====================================================================

void initColorPair(void);
bool doEscKey(WINDOW* window);
bool doEscKeyW(WINDOW* window);
std::string getDiskType(const FdxHeader& header);

// =====================================================================

};


#endif  // __FDSCOMMON_H__
// =====================================================================
// [EOF]
