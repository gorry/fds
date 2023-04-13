// ◇
// fds: fdx68 selector
// FDSCommon: FDS共通
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSCommon.h"

// =====================================================================
// FDSCommon: FDS共通
// =====================================================================

namespace fds {

// -------------------------------------------------------------
// カラーペア初期化
// -------------------------------------------------------------

void initColorPair(void)
{
	// カラー設定
	char* env = getenv("TERM");
	if (env && strstr(env, "256color")) {
		use_default_colors();
		#define MyColor(r,g,b) (((r)*36)+((g)*6)+(b)+16)
		init_pair((short)ColorPair::Normal,                MyColor(5,5,5), MyColor(0,0,0));
		init_pair((short)ColorPair::FilerUnknown,          MyColor(3,3,3), MyColor(0,0,1));
		init_pair((short)ColorPair::FilerParentDir,        MyColor(0,4,5), MyColor(0,0,1));
		init_pair((short)ColorPair::FilerDir,              MyColor(0,5,0), MyColor(0,0,1));
		init_pair((short)ColorPair::FilerFdxFile,          MyColor(5,5,5), MyColor(0,0,1));
		init_pair((short)ColorPair::FilerOtherImageFile,   MyColor(3,4,3), MyColor(0,0,1));
		init_pair((short)ColorPair::FilerOtherFile,        MyColor(3,2,2), MyColor(0,0,1));
		init_pair((short)ColorPair::FilerUnknownCsr,       MyColor(3,3,3), MyColor(0,0,4));
		init_pair((short)ColorPair::FilerParentDirCsr,     MyColor(0,4,5), MyColor(0,0,4));
		init_pair((short)ColorPair::FilerDirCsr,           MyColor(0,5,0), MyColor(0,0,4));
		init_pair((short)ColorPair::FilerFdxFileCsr,       MyColor(5,5,5), MyColor(0,0,4));
		init_pair((short)ColorPair::FilerOtherFileCsr,     MyColor(3,3,3), MyColor(0,0,4));
		init_pair((short)ColorPair::FilerProtected,        MyColor(5,0,0), MyColor(0,0,1));
		init_pair((short)ColorPair::FilerProtectedCsr,     MyColor(5,0,0), MyColor(0,0,4));
		init_pair((short)ColorPair::Header,                MyColor(0,0,0), MyColor(5,3,0));
		init_pair((short)ColorPair::PathHeader,            MyColor(0,4,5), MyColor(0,0,0));
		init_pair((short)ColorPair::PathRoot,              MyColor(3,5,0), MyColor(0,0,0));
		init_pair((short)ColorPair::PathCurrent,           MyColor(0,5,3), MyColor(0,0,0));
		init_pair((short)ColorPair::SelectHeader,          MyColor(0,4,5), MyColor(0,0,0));
		init_pair((short)ColorPair::SelectItem,            MyColor(5,5,5), MyColor(0,0,0));
		init_pair((short)ColorPair::SelectItemCursor,      MyColor(5,5,5), MyColor(0,0,4));
		init_pair((short)ColorPair::InputHeader,           MyColor(0,4,5), MyColor(0,0,0));
		init_pair((short)ColorPair::InputEdit,             MyColor(5,5,5), MyColor(0,0,4));
		init_pair((short)ColorPair::FddHeaderOff,          MyColor(0,0,0), MyColor(2,2,2));
		init_pair((short)ColorPair::FddHeaderOn,           MyColor(0,5,0), MyColor(2,2,2));
		init_pair((short)ColorPair::FddProtect,            MyColor(5,0,0), MyColor(0,0,0));
		init_pair((short)ColorPair::FddCluster,            MyColor(0,5,3), MyColor(0,0,0));
		init_pair((short)ColorPair::HelpHeader,            MyColor(0,4,5), MyColor(0,0,0));
		init_pair((short)ColorPair::InfoHeader,            MyColor(0,4,5), MyColor(0,0,0));
		init_pair((short)ColorPair::DumpHeader,            MyColor(0,4,5), MyColor(0,0,0));
		init_pair((short)ColorPair::DumpGauge,             MyColor(0,5,3), MyColor(0,0,0));
		init_pair((short)ColorPair::DumpStatusNone,        MyColor(3,3,3), MyColor(0,0,0));
		init_pair((short)ColorPair::DumpStatusFinish,      MyColor(5,5,5), MyColor(0,0,0));
		init_pair((short)ColorPair::DumpStatusUnformat,    MyColor(0,0,0), MyColor(5,5,5));
		init_pair((short)ColorPair::DumpStatusError,       MyColor(0,4,5), MyColor(5,0,0));
		init_pair((short)ColorPair::DumpStatusWarn,        MyColor(5,5,0), MyColor(0,0,5));
		init_pair((short)ColorPair::DumpStatusCsr,         MyColor(5,5,5), MyColor(0,0,4));
		init_pair((short)ColorPair::RestoreHeader,         MyColor(0,4,5), MyColor(0,0,0));
		init_pair((short)ColorPair::RestoreGauge,          MyColor(0,5,3), MyColor(0,0,0));
		init_pair((short)ColorPair::RestoreStatusNone,     MyColor(3,3,3), MyColor(0,0,0));
		init_pair((short)ColorPair::RestoreStatusFinish,   MyColor(5,5,5), MyColor(0,0,0));
		init_pair((short)ColorPair::RestoreStatusUnformat, MyColor(0,0,0), MyColor(5,5,5));
		init_pair((short)ColorPair::RestoreStatusError,    MyColor(0,4,5), MyColor(5,0,0));
		init_pair((short)ColorPair::TrackNormalSec,        MyColor(5,5,5), MyColor(0,0,1));
		init_pair((short)ColorPair::TrackErrorSec,         MyColor(5,0,0), MyColor(0,0,1));
		init_pair((short)ColorPair::TrackF5F6F7Sec,        MyColor(5,5,0), MyColor(0,0,1));
		init_pair((short)ColorPair::TrackNormalSecCsr,     MyColor(5,5,5), MyColor(0,0,4));
		init_pair((short)ColorPair::TrackErrorSecCsr,      MyColor(5,0,0), MyColor(0,0,4));
		init_pair((short)ColorPair::TrackF5F6F7SecCsr,     MyColor(5,5,0), MyColor(0,0,4));
		init_pair((short)ColorPair::TrackAbnormalItem,     MyColor(5,0,0), MyColor(5,5,5));
		init_pair((short)ColorPair::TrackStatus,           MyColor(5,5,5), MyColor(0,3,0));
		init_pair((short)ColorPair::TrackStatusFM,         MyColor(5,5,5), MyColor(5,0,0));
		init_pair((short)ColorPair::TrackGauge,            MyColor(0,5,3), MyColor(0,0,0));
		init_pair((short)ColorPair::SectorNormal,          MyColor(5,5,5), MyColor(0,0,1));
		init_pair((short)ColorPair::SectorNormalCsr,       MyColor(5,5,5), MyColor(0,0,4));
		init_pair((short)ColorPair::SectorBit0,            MyColor(5,0,0), MyColor(5,5,5));
		init_pair((short)ColorPair::SectorBit3,            MyColor(5,0,5), MyColor(5,5,5));
		init_pair((short)ColorPair::SectorBit6,            MyColor(0,0,5), MyColor(5,5,5));
		init_pair((short)ColorPair::SectorBit7,            MyColor(5,0,0), MyColor(5,5,0));
		init_pair((short)ColorPair::SectorBit8,            MyColor(5,0,5), MyColor(5,5,0));
		init_pair((short)ColorPair::SectorBitB,            MyColor(0,0,5), MyColor(5,5,0));
		init_pair((short)ColorPair::SectorGauge,           MyColor(0,5,3), MyColor(0,0,0));

	} else {
		init_pair((short)ColorPair::Normal,                COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::FilerUnknown,          COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::FilerParentDir,        COLOR_YELLOW, COLOR_BLACK);
		init_pair((short)ColorPair::FilerDir,              COLOR_GREEN, COLOR_BLACK);
		init_pair((short)ColorPair::FilerFdxFile,          COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::FilerOtherImageFile,   COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::FilerOtherFile,        COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::FilerUnknownCsr,       COLOR_WHITE, COLOR_BLUE);
		init_pair((short)ColorPair::FilerParentDirCsr,     COLOR_YELLOW, COLOR_BLUE);
		init_pair((short)ColorPair::FilerDirCsr,           COLOR_GREEN, COLOR_BLUE);
		init_pair((short)ColorPair::FilerFdxFileCsr,       COLOR_WHITE, COLOR_BLUE);
		init_pair((short)ColorPair::FilerOtherFileCsr,     COLOR_WHITE, COLOR_BLUE);
		init_pair((short)ColorPair::FilerProtected,        COLOR_RED, COLOR_BLACK);
		init_pair((short)ColorPair::FilerProtectedCsr,     COLOR_RED, COLOR_BLUE);
		init_pair((short)ColorPair::Header,                COLOR_BLACK, COLOR_YELLOW);
		init_pair((short)ColorPair::PathHeader,            COLOR_CYAN, COLOR_BLACK);
		init_pair((short)ColorPair::PathRoot,              COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::PathCurrent,           COLOR_GREEN, COLOR_BLACK);
		init_pair((short)ColorPair::SelectHeader,          COLOR_CYAN, COLOR_BLACK);
		init_pair((short)ColorPair::SelectItem,            COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::SelectItemCursor,      COLOR_WHITE, COLOR_BLUE);
		init_pair((short)ColorPair::InputHeader,           COLOR_CYAN, COLOR_BLACK);
		init_pair((short)ColorPair::InputEdit,             COLOR_WHITE, COLOR_BLUE);
		init_pair((short)ColorPair::FddHeaderOff,          COLOR_BLACK, COLOR_WHITE);
		init_pair((short)ColorPair::FddHeaderOn,           COLOR_GREEN, COLOR_WHITE);
		init_pair((short)ColorPair::FddProtect,            COLOR_RED, COLOR_BLACK);
		init_pair((short)ColorPair::FddCluster,            COLOR_GREEN, COLOR_BLACK);
		init_pair((short)ColorPair::HelpHeader,            COLOR_CYAN, COLOR_BLACK);
		init_pair((short)ColorPair::InfoHeader,            COLOR_CYAN, COLOR_BLACK);
		init_pair((short)ColorPair::DumpHeader,            COLOR_CYAN, COLOR_BLACK);
		init_pair((short)ColorPair::DumpGauge,             COLOR_GREEN, COLOR_BLACK);
		init_pair((short)ColorPair::DumpStatusNone,        COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::DumpStatusFinish,      COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::DumpStatusUnformat,    COLOR_BLACK, COLOR_WHITE);
		init_pair((short)ColorPair::DumpStatusError,       COLOR_WHITE, COLOR_RED);
		init_pair((short)ColorPair::DumpStatusWarn,        COLOR_YELLOW, COLOR_BLUE);
		init_pair((short)ColorPair::DumpStatusCsr,         COLOR_WHITE, COLOR_BLUE);
		init_pair((short)ColorPair::RestoreHeader,         COLOR_CYAN, COLOR_BLACK);
		init_pair((short)ColorPair::RestoreGauge,          COLOR_GREEN, COLOR_BLACK);
		init_pair((short)ColorPair::RestoreStatusNone,     COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::RestoreStatusFinish,   COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::RestoreStatusUnformat, COLOR_BLACK, COLOR_WHITE);
		init_pair((short)ColorPair::RestoreStatusError,    COLOR_WHITE, COLOR_RED);
		init_pair((short)ColorPair::TrackNormalSec,        COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::TrackErrorSec,         COLOR_RED, COLOR_BLACK);
		init_pair((short)ColorPair::TrackF5F6F7Sec,        COLOR_YELLOW, COLOR_BLACK);
		init_pair((short)ColorPair::TrackNormalSecCsr,     COLOR_WHITE, COLOR_BLUE);
		init_pair((short)ColorPair::TrackErrorSecCsr,      COLOR_RED, COLOR_BLUE);
		init_pair((short)ColorPair::TrackF5F6F7SecCsr,     COLOR_YELLOW, COLOR_BLUE);
		init_pair((short)ColorPair::TrackAbnormalItem,     COLOR_RED, COLOR_WHITE);
		init_pair((short)ColorPair::TrackStatus,           COLOR_BLACK, COLOR_GREEN);
		init_pair((short)ColorPair::TrackStatusFM,         COLOR_WHITE, COLOR_RED);
		init_pair((short)ColorPair::TrackGauge,            COLOR_GREEN, COLOR_BLACK);
		init_pair((short)ColorPair::SectorNormal,          COLOR_WHITE, COLOR_BLACK);
		init_pair((short)ColorPair::SectorNormalCsr,       COLOR_WHITE, COLOR_BLUE);
		init_pair((short)ColorPair::SectorBit0,            COLOR_RED, COLOR_WHITE);
		init_pair((short)ColorPair::SectorBit3,            COLOR_MAGENTA, COLOR_WHITE);
		init_pair((short)ColorPair::SectorBit6,            COLOR_BLUE, COLOR_WHITE);
		init_pair((short)ColorPair::SectorBit7,            COLOR_RED, COLOR_YELLOW);
		init_pair((short)ColorPair::SectorBit8,            COLOR_MAGENTA, COLOR_YELLOW);
		init_pair((short)ColorPair::SectorBitB,            COLOR_BLUE,  COLOR_YELLOW);
		init_pair((short)ColorPair::SectorGauge,           COLOR_GREEN, COLOR_BLACK);
	}
}

// -------------------------------------------------------------
// コマンドキー設定
// -------------------------------------------------------------
const ESCKEYMAP sEscKeyMap[] = {
	{ "[11~", 265 },
	{ "[12~", 266 },
	{ "[13~", 267 },
	{ "[14~", 268 },
	{ "[1~", KEY_HOME },
	{ "[4~", KEY_END },
	{ "Ol", '+' },
	{ "On", '.' },
	{ "Op", '0' },
	{ "Oq", '1' },
	{ "Or", '2' },
	{ "Os", '3' },
	{ "Ot", '4' },
	{ "Ou", '5' },
	{ "Ov", '6' },
	{ "Ow", '7' },
	{ "Ox", '8' },
	{ "Oy", '9' },
};

// -------------------------------------------------------------
// ESCキー（と各コマンドキー）入力時の処理
// -------------------------------------------------------------
bool
doEscKey(WINDOW* window)
{
	std::string keys;
	int key2 = wgetch(window);
	if (key2 == ERR) {
		return true;
	} else {
		keys.push_back((char)key2);
		while ((key2 = wgetch(window)) != ERR) {
			keys.push_back((char)key2);
		}
		for (int i=0; i<(int)_countof(sEscKeyMap); i++) {
			if (keys == sEscKeyMap[i].keys) {
				ungetch(sEscKeyMap[i].newkey);
				return false;
			}
		}
		FDS_LOG("doEscKey: keys=[%s]\n", keys.c_str());
	}
	return false;
}

// -------------------------------------------------------------
// ESCキー（と各コマンドキー）入力時の処理: ワイド用
// -------------------------------------------------------------
bool
doEscKeyW(WINDOW* window)
{
	std::wstring wkeys;
	wint_t wch;
	int wkey = wget_wch(window, &wch);
	if (wkey == ERR) {
		return true;
	} else {
		wkeys.push_back((wchar_t)wch);
		while ((wkey = wget_wch(window, &wch)) != ERR) {
			wkeys.push_back((wchar_t)wch);
		}
		std::string keys = WStrUtil::wstr2str(wkeys);
		for (int i=0; i<(int)_countof(sEscKeyMap); i++) {
			if (keys == sEscKeyMap[i].keys) {
				unget_wch((wchar_t)sEscKeyMap[i].newkey);
				return false;
			}
		}
		FDS_LOG("doEscKeyW: wkeys=[%ls]\n", wkeys.c_str());
	}
	return false;
}


// -------------------------------------------------------------
// ディスクタイプ文字列を返す
// -------------------------------------------------------------
std::string
getDiskType(const FdxHeader& header)
{
	// ディスクタイプ判別
	std::string disktype = "???";

	switch (header.mType) {
	  case 0:
		disktype = "2D";
		break;
	  case 1:
		disktype = ((header.mRpm == 300) ? "2DD(720KB)" : "2DD(640KB)");
		break;
	  case 2:
		disktype = ((header.mRpm == 300) ? "2HD(1.4MB,PC/AT)" : "2HD(1.2MB,JP)");
		break;
	  case 9:
		if (header.mCylinders < 60) {
			disktype = "2D[RAW]";
		} else {
			if (header.mRate < 5000) {
				disktype = ((header.mRpm == 300) ? "2DD(720KB)[RAW]" : "2DD(640KB)[RAW]");
			} else {
				disktype = ((header.mRpm == 300) ? "2HD(1.4MB,PC/AT)[RAW]" : "2HD(1.2MB,JP)[RAW]");
			}
		}
		break;
	  default:
		break;
	}

	return disktype;
}

// =====================================================================

};

// =====================================================================
// [EOF]
