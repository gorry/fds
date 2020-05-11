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

typedef struct {
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
} FdxHeader;


#endif  // __FDXFILE_H__
// =====================================================================
// [EOF]
