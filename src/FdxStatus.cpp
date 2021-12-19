// ◇
// fds: fdx68 selector
// FdxView: Fdxステータス
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FdxStatus.h"

// -------------------------------------------------------------
// トラックが読み込み済みか調べる
// -------------------------------------------------------------
bool
FdxDiskInfo::isTrackReady(int trackno)
{
	// トラック範囲チェック
	if ((trackno < 0) || ((int)TrackSize() <= trackno)) {
		return false;
	}

	// トラックが読み込み済みなら即終了
	if (Track(trackno).mLastSectorNo < 0) {
		return false;
	}

	return true;
}

// -------------------------------------------------------------
// トラックのリロードを要求
// -------------------------------------------------------------
bool
FdxDiskInfo::ReqTrackReload(int trackno)
{
	// トラック範囲チェック
	if ((trackno < 0) || ((int)TrackSize() <= trackno)) {
		return false;
	}

	// トラックの情報をクリア
	Track(trackno).mLastSectorNo = -1;

	return true;
}

// -------------------------------------------------------------
// セクタが読み込み済みか調べる
// -------------------------------------------------------------
bool
FdxTrackInfo::isSectorReady(int sectorno)
{
	// セクタ範囲チェック
	if ((sectorno < 0) || ((int)SectorSize() <= sectorno)) {
		return false;
	}

	// セクタが読み込み済みなら即終了
	if (Sector(sectorno).mSectorData.mOffset < 0) {
		return false;
	}

	return true;
}

bool
FdxDiskInfo::isSectorReady(int trackno, int sectorno)
{
	// トラック範囲チェック
	if ((trackno < 0) || ((int)TrackSize() <= trackno)) {
		return false;
	}

	return Track(trackno).isSectorReady(sectorno);
}

// -------------------------------------------------------------
// セクタのリロードを要求
// -------------------------------------------------------------
bool
FdxTrackInfo::ReqSectorReload(int sectorno)
{
	// セクタ範囲チェック
	if ((sectorno < 0) || ((int)SectorSize() <= sectorno)) {
		return false;
	}

	// セクタの情報をクリア
	Sector(sectorno).mSectorData.mOffset = -1;

	return true;
}

bool
FdxDiskInfo::ReqSectorReload(int trackno, int sectorno)
{
	// トラック範囲チェック
	if ((trackno < 0) || ((int)TrackSize() <= trackno)) {
		return false;
	}

	return Track(trackno).ReqSectorReload(sectorno);
}

// =====================================================================
// [EOF]
