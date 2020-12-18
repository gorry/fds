// ◇
// fds: fdx68 selector
// FDSSystem: ダンプ
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSSystem.h"

// =====================================================================
// FDSSystem: ダンプ
// =====================================================================

// -------------------------------------------------------------
// ビュー作成
// -------------------------------------------------------------
void
FDSSystem::dumpViewCreateWindow()
{
	if (mwDumpView) {
		dumpViewDestroyWindow();
	}
	mwDumpView = newwin(
	  mDumpViewXYWH.h(),
	  mDumpViewXYWH.w(),
	  mDumpViewXYWH.y(),
	  mDumpViewXYWH.x()
	);
}

// -------------------------------------------------------------
// ビュー破棄
// -------------------------------------------------------------
void
FDSSystem::dumpViewDestroyWindow()
{
	if (mwDumpView) {
		delwin(mwDumpView);
		mwDumpView = nullptr;
	}
}

// -------------------------------------------------------------
// ビュー再描画
// -------------------------------------------------------------
void
FDSSystem::dumpViewRedraw()
{
	redrawwin(mwDumpView);
	dumpViewRefresh();
}

// -------------------------------------------------------------
// ビュー更新
// -------------------------------------------------------------
void
FDSSystem::dumpViewRefresh()
{
	// ビューのクリア
	werase(mwDumpView);

	// ヘッダ
	char buf[FDX_STRING_MAX];
	sprintf(buf, "[Dump %s : %s]", mDumpViewStatus.mName.c_str(), mDumpViewStatus.mFormat.c_str());
	wattron(mwDumpView, COLOR_PAIR(ColorPair::DumpHeader)|A_BOLD);
	std::wstring wbuf = WStrUtil::str2wstr(buf);
	mvwaddwstr(mwDumpView, 1, 2, wbuf.c_str());
	wattroff(mwDumpView, COLOR_PAIR(ColorPair::DumpHeader)|A_BOLD);

	// ゲージ
	wattron(mwDumpView, COLOR_PAIR(ColorPair::DumpGauge));
	//                           01234567890123456789012345678901234567890123456789
	mvwaddwstr(mwDumpView,  2, 2, L"   | 0 1 2 3 4 5 6 7 8 9 | 0 1 2 3 4 5 6 7 8 9 |");
	mvwaddwstr(mwDumpView,  3, 2, L"---+---------------------+---------------------+");
	mvwaddwstr(mwDumpView,  4, 2, L"  0|                     |                     |");
	mvwaddwstr(mwDumpView,  5, 2, L" 20|                     |                     |");
	mvwaddwstr(mwDumpView,  6, 2, L" 40|                     |                     |");
	mvwaddwstr(mwDumpView,  7, 2, L" 60|                     |                     |");
	mvwaddwstr(mwDumpView,  8, 2, L" 80|                     |                     |");
	mvwaddwstr(mwDumpView,  9, 2, L"100|                     |                     |");
	mvwaddwstr(mwDumpView, 10, 2, L"120|                     |                     |");
	mvwaddwstr(mwDumpView, 11, 2, L"140|                     |                     |");
	mvwaddwstr(mwDumpView, 12, 2, L"160|                     |                     |");
	mvwaddwstr(mwDumpView, 13, 2, L"---+---------------------+---------------------+");
	mvwaddwstr(mwDumpView, 14, 2, L"           [Push SPACE Key to Cancel]           ");
	wattroff(mwDumpView, COLOR_PAIR(ColorPair::DumpGauge));

	for (int i=0; i<mDumpViewStatus.mTracks; i++) {
		if (!0) { // mDumpViewStatus.mChanged[i]) {
			int n = i%20;
			int x =  (n+(n>=10)) * 2 + 7;
			int y = (i/20) + 4;
			switch (mDumpViewStatus.mStatus[i]) {
			  default:
				break;
			  case FdDump::TrackStatus::None:
				wattron(mwDumpView, COLOR_PAIR(ColorPair::DumpStatusNone));
				mvwaddch(mwDumpView, y, x, '.');
				wattroff(mwDumpView, COLOR_PAIR(ColorPair::DumpStatusNone));
				break;
			  case FdDump::TrackStatus::Retry1:
			  case FdDump::TrackStatus::Retry2:
			  case FdDump::TrackStatus::Retry3:
			  case FdDump::TrackStatus::Retry4:
			  case FdDump::TrackStatus::Retry5:
			  case FdDump::TrackStatus::Retry6:
			  case FdDump::TrackStatus::Retry7:
			  case FdDump::TrackStatus::Retry8:
			  case FdDump::TrackStatus::Retry9:
				wattron(mwDumpView, COLOR_PAIR(ColorPair::DumpStatusFinish));
				mvwaddch(mwDumpView, y, x, '0'+(int)mDumpViewStatus.mStatus[i]);
				wattroff(mwDumpView, COLOR_PAIR(ColorPair::DumpStatusFinish));
				break;
			  case FdDump::TrackStatus::Finish:
				wattron(mwDumpView, COLOR_PAIR(ColorPair::DumpStatusFinish));
				mvwaddch(mwDumpView, y, x, 'o');
				wattroff(mwDumpView, COLOR_PAIR(ColorPair::DumpStatusFinish));
				break;
			  case FdDump::TrackStatus::Error:
				wattron(mwDumpView, COLOR_PAIR(ColorPair::DumpStatusError));
				mvwaddch(mwDumpView, y, x, 'E');
				wattroff(mwDumpView, COLOR_PAIR(ColorPair::DumpStatusError));
				break;
			  case FdDump::TrackStatus::Unformat:
				wattron(mwDumpView, COLOR_PAIR(ColorPair::DumpStatusUnformat));
				mvwaddch(mwDumpView, y, x, '-');
				wattroff(mwDumpView, COLOR_PAIR(ColorPair::DumpStatusUnformat));
				break;
			}
		}
	}

	// 枠を追加して更新
	wborder(mwDumpView, 0,0,0,0,0,0,0,0);
	wrefresh(mwDumpView);
}

// -------------------------------------------------------------
// ビュー状態更新
// -------------------------------------------------------------
void
FDSSystem::dumpViewUpdate(FdDump::Status& st)
{
	mDumpViewStatus = st;
	memset(st.mChanged, 0, sizeof(st.mChanged));
}


// [EOF]
