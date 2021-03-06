﻿// ◇
// fds: fdx68 selector
// FdxView: FdxViewの操作
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt

#if !defined(__FDXVIEW_H__)
#define __FDXVIEW_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>

#if !defined(FDS_WINDOWS)
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <windows.h>
#include <io.h>
#endif

#include <vector>
#include <string>

#include "WStrUtil.h"
#include "FDXFile.h"
#include "Log.h"

// =====================================================================
// FdxViewの操作
// =====================================================================

class FdxView
{
public:		// struct, enum
	class FdxStatus {
	  public:
		enum class Status : uint32_t {
			None           = 0,
			ErrMask        = 0xff000000,
			ErrInfoCRC     = 0x40000000,
			ErrInfoInvalid = 0x20000000,
			ErrDataCRC     = 0x10000000,
			ErrDataInvalid = 0x08000000,
			ErrDataNothing = 0x04000000,
			ErrDataDeleted = 0x02000000,
			ErrUnknown     = 0x01000000,
			InfoMask       = 0x00ff0000,
			InfoISTR       = 0x00800000,
			InfoFMMask     = 0x00600000,
			InfoMIX        = 0x00600000,
			InfoMFM        = 0x00400000,
			InfoFM         = 0x00200000,
			InfoIAM        = 0x00100000,
			SecMask        = 0x000000ff,
		};
		FdxStatus() { mStatus = Status::None; }
		FdxStatus(Status s) { mStatus = s; }
		Status& operator = (const Status& s) { mStatus = s; return mStatus; }
		uint32_t n(void) const { return (uint32_t)mStatus; }

		bool Err(void) const { return ((uint32_t)mStatus & (uint32_t)Status::ErrMask) != 0; }
		bool ErrInfoCRC(void) const { return ((uint32_t)mStatus & (uint32_t)Status::ErrInfoCRC) != 0; }
		bool ErrInfoInvalid(void) const { return ((uint32_t)mStatus & (uint32_t)Status::ErrInfoInvalid) != 0; }
		bool ErrDataCRC(void) const { return ((uint32_t)mStatus & (uint32_t)Status::ErrDataCRC) != 0; }
		bool ErrDataInvalid(void) const { return ((uint32_t)mStatus & (uint32_t)Status::ErrDataInvalid) != 0; }
		bool ErrDataNothing(void) const { return ((uint32_t)mStatus & (uint32_t)Status::ErrDataNothing) != 0; }
		bool ErrDataDeleted(void) const { return ((uint32_t)mStatus & (uint32_t)Status::ErrDataDeleted) != 0; }
		bool InfoISTR(void) const { return ((uint32_t)mStatus & (uint32_t)Status::InfoISTR) != 0; }
		bool InfoIMIX(void) const { return ((uint32_t)mStatus & (uint32_t)Status::InfoFMMask) == (uint32_t)Status::InfoFMMask; }
		bool InfoIMFM(void) const { return ((uint32_t)mStatus & (uint32_t)Status::InfoMFM) != 0; }
		bool InfoIFM(void) const { return ((uint32_t)mStatus & (uint32_t)Status::InfoFM) != 0; }
		bool InfoIAM(void) const { return ((uint32_t)mStatus & (uint32_t)Status::InfoIAM) != 0; }
		int  Sec(void) const { return (uint32_t)mStatus & (uint32_t)Status::SecMask; }
		void SetErr(int s) { mStatus = (Status)(((uint32_t)mStatus & (~(uint32_t)Status::ErrMask)) | (uint32_t)((s&0xff) << 24)); };
		void SetErrInfoCRC(bool s) { mStatus = (Status)(((uint32_t)mStatus & (~(uint32_t)Status::ErrInfoCRC)) | (uint32_t)(s ? Status::ErrInfoCRC : Status::None)); }
		void SetErrInfoInvalid(bool s) { mStatus = (Status)(((uint32_t)mStatus & (~(uint32_t)Status::ErrInfoInvalid)) | (uint32_t)(s ? Status::ErrInfoInvalid : Status::None)); }
		void SetErrDataCRC(bool s) { mStatus = (Status)(((uint32_t)mStatus & (~(uint32_t)Status::ErrDataCRC)) | (uint32_t)(s ? Status::ErrDataCRC : Status::None)); }
		void SetErrDataInvalid(bool s) { mStatus = (Status)(((uint32_t)mStatus & (~(uint32_t)Status::ErrDataInvalid)) | (uint32_t)(s ? Status::ErrDataInvalid : Status::None)); }
		void SetErrDataNothing(bool s) { mStatus = (Status)(((uint32_t)mStatus & (~(uint32_t)Status::ErrDataNothing)) | (uint32_t)(s ? Status::ErrDataNothing : Status::None)); }
		void SetErrDataDeleted(bool s) { mStatus = (Status)(((uint32_t)mStatus & (~(uint32_t)Status::ErrDataDeleted)) | (uint32_t)(s ? Status::ErrDataDeleted : Status::None)); }

		void SetInfoISTR(bool s) { mStatus = (Status)(((uint32_t)mStatus & (~(uint32_t)Status::InfoISTR)) | (uint32_t)(s ? Status::InfoISTR : Status::None)); }
		void SetInfoIMIX(bool s) { mStatus = (Status)(((uint32_t)mStatus & (~(uint32_t)Status::InfoFMMask)) | (uint32_t)(s ? Status::InfoMIX : Status::None)); }
		void SetInfoIMFM(bool s) { mStatus = (Status)(((uint32_t)mStatus & (~(uint32_t)Status::InfoFMMask)) | (uint32_t)(s ? Status::InfoMFM : Status::None)); }
		void SetInfoIFM(bool s) { mStatus = (Status)(((uint32_t)mStatus & (~(uint32_t)Status::InfoFMMask)) | (uint32_t)(s ? Status::InfoFM : Status::None)); }
		void SetInfoIAM(bool s) { mStatus = (Status)(((uint32_t)mStatus & (~(uint32_t)Status::InfoIAM)) | (uint32_t)(s ? Status::InfoIAM : Status::None)); }
		void SetInfoSec(int s) { mStatus = (Status)(((uint32_t)mStatus & (~(uint32_t)Status::SecMask)) | (uint32_t)(s & 0xff)); }

	  private:
		Status mStatus;
	};
	class SectorData {
	  public:
		std::vector<uint8_t> mData;
		std::vector<uint8_t> mEncode;
		int mOffset;
	  public:
		SectorData() { Clear(); }
		virtual ~SectorData() {}
		void Clear(void) {
			mData.clear();
			mEncode.clear();
			mOffset = -1;
		}
		std::vector<uint8_t>& Data(void) { return mData; }
		std::vector<uint8_t>& Encode(void) { return mEncode; }
	};
	class SectorInfo {
	  public:
		int mCellStart;
		int mCellEnd;
		int mTime;
		uint32_t mCHRN;
		int mSecSize;
		int mGap2;
		int mGap3;
		FdxStatus mStatus;
		SectorData mSectorData;
		double mDRate;
	  public:
		SectorInfo() { Clear(); }
		virtual ~SectorInfo() {}
		// SectorInfo(SectorInfo&& o) = default;
		void Clear(void) {
			mCellStart = -1;
			mCellEnd = 0;
			mTime = 0;
			mCHRN = 0;
			mSecSize = 0;
			mGap2 = 0;
			mGap3 = 0;
			mStatus = FdxStatus::Status::None;
			mDRate = 0.0;
		}
		std::vector<uint8_t>& Data(void) { return mSectorData.Data(); }
		std::vector<uint8_t>& Encode(void) { return mSectorData.Encode(); }
		bool SecF5F6F7(void) const { int r = CHRN_R(); return ((r==0xf5)||(r==0xf6)||(r==0xf7)); }
		int CHRN_C(void) const { return (mCHRN>>24)&0xff; }
		int CHRN_H(void) const { return (mCHRN>>16)&0xff; }
		int CHRN_R(void) const { return (mCHRN>>8)&0xff; }
		int CHRN_N(void) const { return (mCHRN>>0)&0xff; }
		void SetCHRN(int c, int h, int r, int n) { mCHRN = ((c&0xff)<<24) | ((h&0xff)<<16) | ((r&0xff)<<8) | ((n&0xff)<<0); }
		void SetCHRN_C(int s) { mCHRN = (mCHRN&0x00ffffff) | ((s&0xff)<<24); }
		void SetCHRN_H(int s) { mCHRN = (mCHRN&0xff00ffff) | ((s&0xff)<<16); }
		void SetCHRN_R(int s) { mCHRN = (mCHRN&0xffff00ff) | ((s&0xff)<<8); }
		void SetCHRN_N(int s) { mCHRN = (mCHRN&0xffffff00) | ((s&0xff)<<0); }
	};

	class TrackInfo {
	  public:
		int mCylinder;
		int mHead;
		int mLenBits;
		double mDRate;
		int mSectors;
		int mSizeGap4a;
		int mCellIAM;
		int mSizeGap1;
		int mSizeGap4b;
		FdxStatus mStatus;
		std::vector<SectorInfo> mSector;
		int mLastSectorNo;

	  public:
		TrackInfo() { Clear(); }
		virtual ~TrackInfo() {}
		// TrackInfo(TrackInfo&& o) = default;
		void Clear(void) {
			mCylinder = 0;
			mHead = 0;
			mLenBits = 0;
			mDRate = 0.0;
			mSectors = 0;
			mSizeGap4a = -1;
			mCellIAM = -1;
			mSizeGap1 = -1;
			mSizeGap4b = -1;
			mStatus = FdxView::FdxStatus::Status::None;
			mSector.clear();
			mLastSectorNo = -1;
		}
		size_t SectorSize(void) const { return mSector.size(); }
		SectorInfo& Sector(int sectorno) { return mSector[sectorno]; }
		bool isSectorReady(int sectorno);
		bool ReqSectorReload(int sectorno);
	};

	class DiskInfo {
	  public:
		FdxHeader mFdxInfo;
		std::vector<TrackInfo> mTrack;
		int mLastTrackNo;

	  public:
		DiskInfo() { Clear(); }
		void Clear(void) {
			mTrack.clear();
			mLastTrackNo = -1;
		}
		size_t TrackSize(void) const { return mTrack.size(); }
		TrackInfo& Track(int trackno) { return mTrack[trackno]; }
		bool isTrackReady(int trackno);
		bool ReqTrackReload(int trackno);
		bool isSectorReady(int trackno, int sectorno);
		bool ReqSectorReload(int trackno, int sectorno);
	};

public:		// function
	FdxView() { Clear(); }
	virtual ~FdxView() {}
	FdxView(FdxView&& o) = default;
	void Clear(void) {
	}

	bool execCmd(const std::string& cmd, const std::string& option);

	bool readFDXDiskInfo(const std::string& cmd, const std::string& filename);
	bool readFDXTrack(const std::string& cmd, const std::string& filename, int trackno);
	bool readFDXSector(const std::string& cmd, const std::string& filename, int trackno, int sectorno);

	DiskInfo& diskInfo(void) { return mDiskInfo; }
	bool isTrackReady(int trackno);
	bool ReqTrackReload(int trackno);
	TrackInfo& Track(int trackno) { return mDiskInfo.Track(trackno); }
	SectorInfo& Sector(int trackno, int sectorno) { return mDiskInfo.Track(trackno).Sector(sectorno); }
	bool isSectorReady(int trackno, int sectorno);
	bool ReqSectorReload(int trackno, int sectorno);

private:	// function
	int readFDXDiskInfoHeader(DiskInfo& diskinfo, char* s);
	int readFDXDiskInfoBody(DiskInfo& diskinfo, char* s);
	int readFDXDiskInfoDump(DiskInfo& diskinfo, char* s);
	bool execFdxViewAnalyze(char* tmpfilename, const std::string& cmd, const std::string& filename, const char* option);


public:		// var
	DiskInfo mDiskInfo;
	// DiskInfo mTrackInfo;

private:	// var

};

#endif  // __FDXVIEW_H__
// =====================================================================
// [EOF]
