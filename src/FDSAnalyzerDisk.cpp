// ◇
// fds: fdx68 selector
// FDSAnalyzer: FDSアナライザ:ディスク
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSAnalyzer.h"

// =====================================================================
// FDSAnalyzer: FDSアナライザ:ディスク
// =====================================================================

// -------------------------------------------------------------
// ビュー作成
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewCreateWindow()
{
	if (mwDiskView) {
		diskViewDestroyWindow();
	}
	mwDiskView = newwin(
	  mDiskViewXYWH.h(),
	  mDiskViewXYWH.w(),
	  mDiskViewXYWH.y(),
	  mDiskViewXYWH.x()
	);
	keypad(mwDiskView, TRUE);

	mDiskViewInnerW = mDiskViewXYWH.w()-2;
	mDiskViewInnerH = mDiskViewXYWH.h()-2;
	mDiskViewCsr = 0;
	mDiskViewWindowOfsX = 1;
	mDiskViewWindowOfsY = 1;
}

// -------------------------------------------------------------
// ビュー破棄
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewDestroyWindow()
{
	if (mwDiskView) {
		delwin(mwDiskView);
		mwDiskView = nullptr;
	}
}

// -------------------------------------------------------------
// ビュー再描画
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewRedraw()
{
	redrawwin(mwDiskView);
	diskViewRefresh();
}

// -------------------------------------------------------------
// ビュー更新
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewRefresh()
{
	// ビューのクリア
	werase(mwDiskView);

	// ゲージ
	wattron(mwDiskView, COLOR_PAIR(fds::ColorPair::DumpGauge));
	//                            012345678901234567890123456789012345678901234567890
	mvwaddstr(mwDiskView,  1, 2, "   | 0 1 2 3 4 5 6 7 8 9 | 0 1 2 3 4 5 6 7 8 9 ");
	mvwaddstr(mwDiskView,  2, 2, "  0|                     |                     ");
	mvwaddstr(mwDiskView,  3, 2, " 20|                     |                     ");
	mvwaddstr(mwDiskView,  4, 2, " 40|                     |                     ");
	mvwaddstr(mwDiskView,  5, 2, " 60|                     |                     ");
	mvwaddstr(mwDiskView,  6, 2, " 80|                     |                     ");
	mvwaddstr(mwDiskView,  7, 2, "100|                     |                     ");
	mvwaddstr(mwDiskView,  8, 2, "120|                     |                     ");
	mvwaddstr(mwDiskView,  9, 2, "140|                     |                     ");
	mvwaddstr(mwDiskView, 10, 2, "160|                     |                     ");
	wattroff(mwDiskView, COLOR_PAIR(fds::ColorPair::DumpGauge));

	FdxTool::DiskInfo& disk = mFdxTool.diskInfo();
	for (int i=0; i<(int)disk.TrackSize(); i++) {
		int n = i%20;
		int x =  (n+(n>=10)) * 2 + 7;
		int y = (i/20) + 2;

		if (i == mDiskViewCsr) {
			wattron(mwDiskView, COLOR_PAIR(fds::ColorPair::DumpStatusCsr));
			mvwaddstr(mwDiskView, y, x-1, "[ ]");
			wattroff(mwDiskView, COLOR_PAIR(fds::ColorPair::DumpStatusCsr));
		}
		int sec = disk.Track(i).mSectors;
		if (sec == 0) {
			wattron(mwDiskView, COLOR_PAIR(fds::ColorPair::DumpStatusUnformat));
			mvwaddch(mwDiskView, y, x, '-');
			wattroff(mwDiskView, COLOR_PAIR(fds::ColorPair::DumpStatusUnformat));
			continue;
		}
		if (sec > 36) {
			sec = 36;
		}
		const char* secstr = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ@";
		if (disk.Track(i).mStatus.Err()) {
			wattron(mwDiskView, COLOR_PAIR(fds::ColorPair::DumpStatusError));
			mvwaddch(mwDiskView, y, x, secstr[sec]);
			wattroff(mwDiskView, COLOR_PAIR(fds::ColorPair::DumpStatusUnformat));
			continue;
		}
		wattron(mwDiskView, COLOR_PAIR(fds::ColorPair::DumpStatusFinish));
		mvwaddch(mwDiskView, y, x, secstr[sec]);
		wattroff(mwDiskView, COLOR_PAIR(fds::ColorPair::DumpStatusFinish));
	}

	// 選択内容に応じてヘルプビューを更新
	helpViewRefresh();

	// 枠を追加して更新
	wborder(mwDiskView, 0,0,0,0,0,0,ACS_LTEE,ACS_BTEE);
	wrefresh(mwDiskView);

}

// -------------------------------------------------------------
// ディスクビューカーソル位置を設定
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewSetCsr(int x, int y)
{
	int idx = x + y*20;
	diskViewSetIdx(idx);
}

// -------------------------------------------------------------
// ファイルリストidxから表示開始位置とカーソル位置を設定
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewSetIdx(int idx)
{
	// カーソルがビュー先頭より前を差さないよう調整
	if (idx < 0) {
		idx = 0;
	}

	// カーソルがビュー最終位置として差せる位置を算出
	FdxTool::DiskInfo& disk = mFdxTool.diskInfo();
	int h = disk.TrackSize()-1;
	if (idx > h) {
		idx = h;
	}

	mDiskViewCsr = idx;
}

// -------------------------------------------------------------
// カーソルを上へ
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewUpCursor()
{
	int i = mDiskViewCsr;
	i -= 20;

	// カーソルがビュー先頭より前を差さないよう調整
	if (i < 0) {
		i += 20;
	}

	// カーソルがビュー最終位置として差せる位置を算出
	FdxTool::DiskInfo& disk = mFdxTool.diskInfo();
	int h = disk.TrackSize()-1;
	if (i > h) {
		i = h;
	}

	mDiskViewCsr = i;
}

// -------------------------------------------------------------
// カーソルを下へ
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewDownCursor()
{
	int i = mDiskViewCsr;
	i += 20;

	// カーソルがビュー末尾より後を差さないよう調整
	FdxTool::DiskInfo& disk = mFdxTool.diskInfo();
	int h = disk.TrackSize()-1;
	if (i > h) {
		i -= 20;
	}

	// カーソルがビュー先頭より前を差さないよう調整
	if (i < 0) {
		i = 0;
	}

	mDiskViewCsr = i;
}

// -------------------------------------------------------------
// カーソルを左へ
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewLeftCursor()
{
	int i = mDiskViewCsr;
	i--;

	// カーソルがビュー先頭より前を差さないよう調整
	if (i < 0) {
		i = 0;
	}

	// カーソルがビュー最終位置として差せる位置を算出
	FdxTool::DiskInfo& disk = mFdxTool.diskInfo();
	int h = disk.TrackSize()-1;
	if (i > h) {
		i = h;
	}

	mDiskViewCsr = i;
}

// -------------------------------------------------------------
// カーソルを右へ
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewRightCursor()
{
	int i = mDiskViewCsr;
	i++;

	// カーソルがビュー末尾より後を差さないよう調整
	FdxTool::DiskInfo& disk = mFdxTool.diskInfo();
	int h = disk.TrackSize()-1;
	if (i > h) {
		i = h;
	}

	// カーソルがビュー先頭より前を差さないよう調整
	if (i < 0) {
		i = 0;
	}

	mDiskViewCsr = i;
}

// -------------------------------------------------------------
// ビューを１ページ前へ
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewPageUpCursor()
{
	diskViewPageTopCursor();
}

// -------------------------------------------------------------
// ビューを１ページ後へ
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewPageDownCursor()
{
	diskViewPageBottomCursor();
}

// -------------------------------------------------------------
// ビューをトラックリスト先頭へ
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewPageTopCursor()
{
	mDiskViewCsr = 0;
}

// -------------------------------------------------------------
// ビューをトラックリスト末尾へ
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewPageBottomCursor()
{
	FdxTool::DiskInfo& disk = mFdxTool.diskInfo();
	int h = disk.TrackSize()-1;

	mDiskViewCsr = h;
}

// -------------------------------------------------------------
// ファイルを選択
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewSelectEntry()
{
}

// -------------------------------------------------------------
// ディスクビューで選択したトラックのidxを得る
// -------------------------------------------------------------
int
FDSAnalyzer::diskViewGetIdx()
{
	int idx = mDiskViewCsr;
	FdxTool::DiskInfo& disk = mFdxTool.diskInfo();
	int h = disk.TrackSize()-1;
	if (idx >= h) {
		idx = h;
	}

	return idx;
}

// -------------------------------------------------------------
// ディスクビューで選択したトラックの情報をトラックビューに表示する
// -------------------------------------------------------------
void
FDSAnalyzer::diskViewShowTrack()
{
	FdxTool::DiskInfo& disk = mFdxTool.diskInfo();
	trackViewSetCylinder(mDiskViewCsr / disk.mFdxInfo.mHeads);
	trackViewSetHead(mDiskViewCsr % disk.mFdxInfo.mHeads);
	trackViewRefresh();
}


// =====================================================================
// [EOF]
