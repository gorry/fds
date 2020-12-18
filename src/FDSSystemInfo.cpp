// ◇
// fds: fdx68 selector
// FDSSystem: FDSシステム:ディスク情報
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSSystem.h"

// =====================================================================
// FDSSystem: FDSシステム:ディスク情報
// =====================================================================

// -------------------------------------------------------------
// ビュー作成
// -------------------------------------------------------------
void
FDSSystem::infoViewCreateWindow()
{
	if (mwInfoView) {
		infoViewDestroyWindow();
	}
	mwInfoView = newwin(
	  mInfoViewXYWH.h(),
	  mInfoViewXYWH.w(),
	  mInfoViewXYWH.y(),
	  mInfoViewXYWH.x()
	);
	keypad(mwInfoView, TRUE);
}

// -------------------------------------------------------------
// ビュー破棄
// -------------------------------------------------------------
void
FDSSystem::infoViewDestroyWindow()
{
	if (mwInfoView) {
		delwin(mwInfoView);
		mwInfoView = nullptr;
	}
}

// -------------------------------------------------------------
// ビュー再描画
// -------------------------------------------------------------
void
FDSSystem::infoViewRedraw()
{
	redrawwin(mwInfoView);
	infoViewRefresh();
}

// -------------------------------------------------------------
// ビュー更新
// -------------------------------------------------------------
void
FDSSystem::infoViewRefresh()
{
	char buf[4][FDX_FILENAME_MAX+4];
	buf[0][0] = '\0';
	buf[1][0] = '\0';
	buf[2][0] = '\0';
	buf[3][0] = '\0';

	// ビューのクリア
	werase(mwInfoView);

	// ディスクヘッダ読み込み
	int ret = infoViewLoadFile();
	if (ret == -1) {
		goto end;
	}
	if (ret == -2) {
		goto showFolderInfo;
	}
	if (ret == -3) {
		goto err_notfdx;
	}
	if (ret == -4) {
		goto err_fdxrevision;
	}

	{
		char buf2[64];
		const char* disktype = "???";

		// ファイル名
		size_t pos = mInfoViewFileName.rfind('/');
		if (pos == std::string::npos) {
			pos = 0;
		} else {
			pos++;
		}
		sprintf(&buf[0][0], "[%s]", &mInfoViewFileName[pos]);

		// ディスクタイプ判別
		memcpy(buf2, &mFdxHeader.mName[0], sizeof(mFdxHeader.mName));
		buf2[sizeof(mFdxHeader.mName)] = '\0';
		switch (mFdxHeader.mType) {
		  case 0:
			disktype = "2D";
			break;
		  case 1:
			disktype = ((mFdxHeader.mRpm == 300) ? "2DD(720KB)" : "2DD(640KB)");
			break;
		  case 2:
			disktype = ((mFdxHeader.mRpm == 300) ? "2HD(1.4MB,PC/AT)" : "2HD(1.2MB,JP)");
			break;
		  case 9:
			if (mFdxHeader.mCylinders < 60) {
				disktype = "2D[RAW]";
			} else {
				if (mFdxHeader.mRate < 5000) {
					disktype = ((mFdxHeader.mRpm == 300) ? "2DD(720KB)[RAW]" : "2DD(640KB)[RAW]");
				} else {
					disktype = ((mFdxHeader.mRpm == 300) ? "2HD(1.4MB,PC/AT)[RAW]" : "2HD(1.2MB,JP)[RAW]");
				}
			}
			break;
		  default:
			break;
		}

		// リビジョン・タイプ・シリンダ・ヘッド
		sprintf(&buf[1][0], "Rev:%d Format:%s Cyl:%d Head:%d", 
		  (int)mFdxHeader.mRevision, disktype, 
		  mFdxHeader.mCylinders,
		  mFdxHeader.mHeads
		);

		// トラックあたりサンプル量・データレート・モーター速度
		sprintf(&buf[2][0], "Smpl/Tr:%d Rate:%dkbps Mtr:%drpm Protect:%s", 
		  mFdxHeader.mTrackSize,
		  mFdxHeader.mRate,
		  mFdxHeader.mRpm,
		  (mFdxHeader.mWriteProtect ? "ON " : "OFF")
		);

		// ディスク名
		sprintf(&buf[3][0], "Disk Name:[%s]",
		  buf2
		);
	}
	goto end;

	// フォルダ情報を取得
  showFolderInfo:;
	if (mInfoViewFileName != "..") {
		// フォルダ内のFDXファイルを取得
		Directory dir(mInfoViewFileName);
		dir.setMaskFdxFile(true);
		dir.getFiles(false);
		dir.sortFiles();

		// フォルダ名
		sprintf(&buf[0][0], "[%s]", mInfoViewFileName.c_str());

		// フォルダ内のFDXファイル
		int n = dir.size();
		if (n > 3) {
			n = 3;
		}
		if (n == 0) {
			sprintf(&buf[1][0], " (no FDX file)");
		} else {
			for (int i=0; i<n; i++) {
				std::wstring wstr = WStrUtil::str2wstr(dir[i].filename());
				std::string str = WStrUtil::wstr2strN(wstr, FDX_FILENAME_MAX);
				sprintf(&buf[i+1][0], " %s", str.c_str());
			}
		}
	}
	goto end;

	// FDXファイルじゃない
  err_notfdx:;
	sprintf(&buf[0][0], "[Not FDX File]");
	goto end;

	// FDXファイルのリビジョンが合わない
  err_fdxrevision:;
	sprintf(&buf[0][0], "[Not Support FDX revision %d]", mFdxHeader.mRevision);
	goto end;

	// 情報を表示
  end:;
	wattron(mwInfoView, COLOR_PAIR(FDSSystem::ColorPair::InfoHeader)|A_BOLD);
	mvwaddstr(mwInfoView, 1, 1, &buf[0][0]);
	wattroff(mwInfoView, COLOR_PAIR(FDSSystem::ColorPair::InfoHeader)|A_BOLD);
	for (int i=1; i<4; i++) {
		mvwaddstr(mwInfoView, i+1, 1, &buf[i][0]);
	}

	// 枠を追加して更新
	wborder(mwInfoView, 0,0,0,0,ACS_LTEE,ACS_RTEE,0,0);
	wrefresh(mwInfoView);
}

// -------------------------------------------------------------
// 情報ビューで扱うファイルを設定
// -------------------------------------------------------------
void
FDSSystem::infoViewSetFile(const std::string& filename)
{
FDS_LOG("infoViewSetFile: filename=[%s]\n", filename.c_str());
	mInfoViewFileName = filename;
}

// -------------------------------------------------------------
// ディスクヘッダの読み込み
// -------------------------------------------------------------
int
FDSSystem::infoViewLoadFile()
{
	// 情報ビューで使うファイルを読み込む
	mDiskInfoResult = 0;
	memset(&mFdxHeader, 0, sizeof(mFdxHeader));
FDS_LOG("infoViewLoadFile: mInfoViewFileName=[%s]\n", mInfoViewFileName.c_str());
	if (mInfoViewFileName.empty()) {
		mDiskInfoResult = -1;
		return mDiskInfoResult;
	}

	{
#if defined(FDS_WINDOWS)
		struct _stat st;
		_stat(mInfoViewFileName.c_str(), &st);
		if (st.st_mode & _S_IFDIR) {
#else
		struct stat st;
		lstat(mInfoViewFileName.c_str(), &st);
		if (st.st_mode & S_IFDIR) {
#endif
			return -2;
		}
	}

	{
		FILE *fin = fopen(mInfoViewFileName.c_str(), "rb");
FDS_LOG("infoViewLoadFile: fin=%p\n", fin);
		if (fin == nullptr) {
			return -2;
		}
		fread(&mFdxHeader, 1, 256, fin);
		fclose(fin);
	}

	// FDXヘッダの確認
	if (memcmp(&mFdxHeader.mSignature, "FDX", 3)) {
		mDiskInfoResult = -3;
		return mDiskInfoResult;
	}

	// FDXリビジョン確認
	if (mFdxHeader.mRevision > 3) {
		mDiskInfoResult = -4;
		return mDiskInfoResult;
	}

	return 0;
}

// -------------------------------------------------------------
// ディスクヘッダの書き込み
// -------------------------------------------------------------
int
FDSSystem::infoViewSaveFile()
{
	// 情報ビューで使うファイルを描き込む
	mDiskInfoResult = 0;
	if (mInfoViewFileName.empty()) {
		return -1;
	}
	{
		FILE *fin = fopen(mInfoViewFileName.c_str(), "w+b");
		if (fin == nullptr) {
			mDiskInfoResult = -1;
			return mDiskInfoResult;
		}
		fwrite(&mFdxHeader, 1, 256, fin);
		fclose(fin);
	}

	return 0;
}

// =====================================================================
// [EOF]
