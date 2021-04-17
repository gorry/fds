// ◇
// fds: fdx68 selector
// FDSSystem: FDXファイル構造
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#if !defined(__FDXFILE_H__)
#define __FDXFILE_H__

// =====================================================================
// FDXファイル構造
// =====================================================================

class FdxHeader {
  public:
	uint8_t mSignature[3];	// $00: 識別子（"FDX"）
	uint8_t mRevision;		// $03: リビジョン（現在は３）
	uint8_t mName[60];		// $04: ディスク名
	uint8_t mPad20[4];		// $20: パディング
	int32_t mType;			// $24: ディスクタイプ（0:2D 1:2DD 2:2HD 9:RAW）
	int32_t mCylinders;		// $28: シリンダ数
	int32_t mHeads;			// $2c: ヘッド数
	int32_t mRate;			// $30: 転送レート（500/1000）
	int32_t mRpm;			// $34: RPM（300/360）
	uint8_t mWriteProtect;	// $38: ライトプロテクト（1でON）
	uint8_t mPad39[3];		// $39: パディング
	uint32_t mOption;		// $3c: 動作オプション
	uint32_t mUnused;		// $40: 未使用
	int32_t mTrackSize;		// $44: トラックデータ長
	uint8_t mReserve[152];	// $48: 構造体サイズ合計256バイト

  public:
	FdxHeader() { Clear(); }
	void Clear(void) {
		memset(&mSignature[0], 0, sizeof(mSignature));
		mRevision = 0;
		memset(mName, 0, sizeof(mName));
		memset(mPad20, 0, sizeof(mPad20));
		mType = 0;
		mCylinders = 0;
		mHeads = 0;
		mRate = 0;
		mRpm = 0;
		mWriteProtect = 0;
		memset(mPad39, 0, sizeof(mPad39));
		mOption = 0;
		mUnused = 0;
		mTrackSize = 0;
		memset(mReserve, 0, sizeof(mReserve));
	}
	bool isRaw(void) { return (mType == 9); }
};


#endif  // __FDXFILE_H__
// =====================================================================
// [EOF]
