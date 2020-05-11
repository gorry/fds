// ◇
// fds: fdx68 selector
// FDSSystem: FDSシステム:コマンド
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "FDSSystem.h"

// =====================================================================
// FDSSystem: FDSシステム:コマンド
// =====================================================================

// -------------------------------------------------------------
// ファイラービューで選択したエントリをリネームする
// -------------------------------------------------------------
void
FDSSystem::cmdRename()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// ディレクトリかFDXファイルのみ受け付ける
	if (!mFiles[idx].isFdxFile() && !mFiles[idx].isNormalDir()) {
		return;
	}

	// 選択したエントリで情報ビューを更新
	std::string path = mRootDir + mCurDir + mFiles[idx].filename();
	if (mFiles[idx].isDir()) {
		path += "/";
	}
	infoViewSetFile(path);
	infoViewRefresh();
	int ret = infoViewGetResult();
	if (ret == -1) {
		return;
	}
	if (ret < -1) {
		// 正常に情報取得できなければ中止
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("FDX File Info Error");
		dlg.setCanEscape(true);
		ret = dlg.start();
		return;
	}

	std::string filename = mFiles[idx].filename();
	while (!0) {
		// 新しいファイル名を入力
		DlgInput dlg;
		dlg.setHeader("Rename to:");
		dlg.setText(filename);
		dlg.setMaxLength(FDX_FILENAME_MAX);
		dlg.setCanEscape(true);
		ret = dlg.start(COLS/2);

		// 入力が行われなかったら中止
		if (ret < 0) {
			return;
		}

		// 入力が空だったら最初の名前でやり直し
		std::string newname = dlg.getText();
		if (newname.empty()) {
			continue;
		}

		// FDXファイル名の入力結果の拡張子が".FDX"でなかったらやり直し
		if (mFiles[idx].isFdxFile()) {
			std::wstring wnewname = WStrUtil::str2wstr(newname);
			size_t len = wnewname.length();
			if ((len < 5) || (WStrUtil::wstricmp(&wnewname[len-4], L".FDX"))) {
				FDS_ERROR("cmdRename: Use Extention \".FDX\"!\n");
				FDS_ERROR(" newname=[%s]\n", newname.c_str());
				DlgSelect dlg2;
				dlg2.setItemsOk();
				dlg2.setHeader("Use Extention \".FDX\"!");
				dlg2.setCanEscape(true);
				dlg2.start();
				refreshAllView();
				filename = newname;
				continue;
			}
		}

		// ファイル名に"/"が含まれていたらやり直し
		if (std::string::npos != newname.find('/')) {
			FDS_ERROR("cmdRename: Can't use '/'!\n");
			FDS_ERROR(" newname=[%s]\n", newname.c_str());
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Can't use '/'!");
			dlg2.setCanEscape(true);
			dlg2.start();
			refreshAllView();
			filename = newname;
			continue;
		}

		// 名前が変わらなかったら中止
		if (mFiles[idx].filename() == newname) {
			return;
		}

		// リネーム
		std::string src = mRootDir + mCurDir + mFiles[idx].filename();
		std::string dst = mRootDir + mCurDir + newname;
		int ret2 = rename(src.c_str(), dst.c_str());
		if (ret2 != 0) {
			FDS_ERROR("cmdRename: Rename Failed!\n");
			FDS_ERROR(" src=[%s], dst=[%s], result=%d, errno=%d\n", src.c_str(), dst.c_str(), ret2, errno);
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Rename Failed!");
			dlg2.setCanEscape(true);
			dlg2.start();
			return;
		}

		// 新しいファイルリストを取得
		mFiles.getFiles(mCurDir.empty());
		mFiles.sortFiles();
		filerViewFindEntry(newname);

		infoViewSetFile(dst);
		infoViewRefresh();
		break;
	}
}

// -------------------------------------------------------------
// ディレクトリ内のイメージを自動挿入する
// ・ファイル名末尾に"1"または"A"が付いたファイルを第１ドライブへ挿入
// ・ファイル名末尾に"2"または"B"が付いたファイルを第２ドライブへ挿入
// ・条件に合うファイルが複数ある場合は、先にリストされているファイルを優先
// ディレクトリを選択している場合は、選択したディレクトリ内のファイルを対象とする
// 選択していない場合は、現在のディレクトリ内のファイルを対象とする
// -------------------------------------------------------------
void
FDSSystem::cmdAutoSet()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// 現在のディレクトリ内容から自動挿入ファイルを選ぶ
	Directory mydir;
	Directory* files = &mFiles;
	std::string path = mRootDir + mCurDir;
	if (mFiles[idx].isNormalDir()) {
		// ディレクトリを選んでいる場合は、選んだディレクトリ内容から自動挿入ファイルを選ぶ
		path += mFiles[idx].filename()+"/";
		mydir.setPath(path);
		mydir.getFiles(true);
		mydir.sortFiles();
		files = &mydir;
	}

	// 全ドライブからイメージを排出し、FDDビューを更新しておく
	for (int i=0; i<FddEmu::Drives; i++) {
		mFddEmu.ejectDrive(i);
	}
	fddViewRefresh();

	// ファイルを検索してドライブへ挿入する
	std::string path2;
	bool setDisk[FddEmu::Drives] = {};
	int disks = 0;
	for (size_t i=0; i<files->size(); i++) {
		if (files->entry(i).isFdxFile()) {
			const std::wstring& s = files->entry(i).wfilename();
			size_t len = s.length();
			wchar_t c = s[len-5];
			switch (c) {
			  case L'1':
			  case L'A':
			  case L'a':
				if (!setDisk[0]) {
					path2 = path + files->entry(i).filename();
					mFddEmu.setImage(0, path2);
					setDisk[0] = true;
					disks++;
				}
				break;
			  case L'2':
			  case L'B':
			  case L'b':
				if (!setDisk[1]) {
					path2 = path + files->entry(i).filename();
					mFddEmu.setImage(1, path2);
					setDisk[1] = true;
					disks++;
				}
				break;
			}
			if (disks >= FddEmu::Drives) break;
		}
	}

	// FDDビューを挿入後の状態で更新する
	fddViewRefresh();

	// １枚もディスクが選ばれなかった場合は警告ダイアログを出しておく
	if (disks == 0) {
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("No Disk Found!");
		dlg.setCanEscape(true);
		dlg.start();
	}
}

// -------------------------------------------------------------
// イメージを挿入する
// -------------------------------------------------------------
void
FDSSystem::cmdSetDrive(int id)
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// FDXファイルのみ受け付ける
	if (!mFiles[idx].isFdxFile()) {
		return;
	}

	// ドライブへ挿入
	std::string src = mRootDir + mCurDir + mFiles[idx].filename();
	mFddEmu.setImage(id, src);

	// FDDビューを挿入後の状態で更新する
	fddViewRefresh();
}

// -------------------------------------------------------------
// 全ドライブのイメージを排出する
// -------------------------------------------------------------
void
FDSSystem::cmdEjectAllDrive()
{
	// 全ドライブのイメージを排出
	for (int i=0; i<FddEmu::Drives; i++) {
		mFddEmu.ejectDrive(i);
	}

	// FDDビューを排出後の状態で更新する
	fddViewRefresh();
}

// -------------------------------------------------------------
// イメージを作成する
// -------------------------------------------------------------
void
FDSSystem::cmdCreateDisk()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// フォーマット選択肢択準備
	DlgSelect::ItemsVec items;
	int n = mIniFile.getInt("GLOBAL", "FORMATS");
	for (int i=0; i<n; i++) {
		char buf[FDX_FILENAME_MAX];
		sprintf(buf, "FORMAT-%d", i+1);
		items.push_back(mIniFile.getString(buf, "NAME"));
	}
	items.push_back("[ Cancel ]");

	// フォーマット選択肢
	int sel = 0;
	while (!0) {
	  format:;
		// ダイアログ表示
		DlgSelect dlg;
		dlg.setItemsVec(items);
		dlg.setHeader("[Select Format]");
		dlg.setCanEscape(true);
		sel = dlg.start(sel);

		// [Cancel]を選んだら終了
		if ((sel == -1) || (sel == (int)items.size()-1)) {
			break;
		}

		// 選択した設定を読み込む
		char section[FDX_FILENAME_MAX];
		sprintf(section, "FORMAT-%d", sel+1);
		std::string name = mIniFile.getString(section, "NAME");
		std::string fdxtoolopt = mIniFile.getString(section, "FDXTOOLOPT");
		std::string filename = mIniFile.getString(section, "FILENAME");

		while (!0) {
			// 新しいファイル名を入力
			DlgInput dlg2;
			dlg2.setHeader("Create ["+name+"] Disk:");
			dlg2.setText(filename);
			dlg2.setCanEscape(true);
			dlg2.setMaxLength(FDX_FILENAME_MAX);
			int ret = dlg2.start(COLS/2);

			// 入力が行われなかったらフォーマット選択に戻る
			if (ret < 0) {
				goto format;
			}

			// 入力が空だったら最初の名前でやり直し
			std::string newname = dlg2.getText();
			if (newname.empty()) {
				refreshAllView();
				continue;
			}
			// 入力結果の拡張子が".FDX"でなかったらやり直し
			std::wstring wnewname = WStrUtil::str2wstr(newname);
			size_t len = wnewname.length();
			if ((len < 5) || (WStrUtil::wstricmp(&wnewname[len-4], L".FDX"))) {
				FDS_ERROR("cmdCreateDisk: Use Extention \".FDX\"!\n");
				FDS_ERROR(" newname=[%s]\n", newname.c_str());
				DlgSelect dlg3;
				dlg3.setItemsOk();
				dlg3.setHeader("Use Extention \".FDX\"!");
				dlg3.setCanEscape(true);
				dlg3.start();
				refreshAllView();
				filename = newname;
				continue;
			}

			// ファイル名に"/"が含まれていたらやり直し
			if (std::string::npos != newname.find('/')) {
				FDS_ERROR("cmdCreateDisk: Can't use '/'!\n");
				FDS_ERROR(" newname=[%s]\n", newname.c_str());
				DlgSelect dlg3;
				dlg3.setItemsOk();
				dlg3.setHeader("Can't use '/'!");
				dlg3.setCanEscape(true);
				dlg3.start();
				refreshAllView();
				filename = newname;
				continue;
			}

			// すでにファイルがあったらやり直し
			std::string dst = mRootDir + mCurDir + newname;
			FILE* fin = fopen(dst.c_str(), "rb");
			if (fin) {
				fclose(fin);
				// 失敗
				FDS_ERROR("cmdCreateDisk: Disk Already Exist!\n");
				FDS_ERROR(" dst=[%s]\n", dst.c_str());
				DlgSelect dlg3;
				dlg3.setItemsOk();
				dlg3.setHeader("Disk Already Exist!");
				dlg3.setCanEscape(true);
				dlg3.start();
				refreshAllView();
				filename = newname;
				continue;
			}

			// イメージを作成
			std::string option = fdxtoolopt+" \""+dst.c_str()+"\"";
			std::string cmd = mIniFile.getString("GLOBAL", "FDXTOOLCMD");
			int ret2 = mFddEmu.execCmd(cmd.c_str(), option.c_str());
			if (ret2 != 0) {
				// 失敗
				FDS_ERROR("cmdCreateDisk: Create Disk Failed!\n");
				FDS_ERROR(" dst=[%s], type=[%s], cmd=[%s], option=[%s], result=%d\n", dst.c_str(), name.c_str(), cmd.c_str(), option.c_str(), ret2);
				DlgSelect dlg3;
				dlg3.setItemsOk();
				dlg3.setHeader("Create Disk Failed!");
				dlg3.setCanEscape(true);
				dlg3.start();
				return;
			}

			// 新しいファイルリストを取得
			mFiles.getFiles(mCurDir.empty());
			mFiles.sortFiles();
			filerViewFindEntry(newname);

			// 作成したファイルの情報を表示
			infoViewSetFile(dst);
			infoViewRefresh();

			return;
		}
	}
}

// -------------------------------------------------------------
// イメージを複製する
// -------------------------------------------------------------
void
FDSSystem::cmdDupDisk()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// FDXファイルのみ受け付ける
	if (!mFiles[idx].isFdxFile()) {
		return;
	}

	// 選択したエントリで情報ビューを更新
	std::string path = mRootDir + mCurDir + mFiles[idx].filename();
	if (mFiles[idx].isDir()) {
		path += "/";
	}
	infoViewSetFile(path);
	infoViewRefresh();
	int ret = infoViewGetResult();
	if (ret == -1) {
		return;
	}
	if (ret < -1) {
		// 正常に情報取得できなければ中止
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("FDX File Info Error");
		dlg.setCanEscape(true);
		dlg.start();
		return;
	}

	std::string filename = mFiles[idx].filename();
	std::string newfile = filename.substr(0, filename.length()-4)+"_Copy.fdx";
	while (!0) {
		// 新しいファイル名を入力
		DlgInput dlg;
		dlg.setHeader("Copy to:");
		dlg.setText(newfile);
		dlg.setMaxLength(FDX_FILENAME_MAX);
		dlg.setCanEscape(true);
		ret = dlg.start(COLS/2);

		// 入力が行われなかったら終了
		if (ret < 0) {
			return;
		}

		// 入力が空だったら最初の名前でやり直し
		std::string newname = dlg.getText();
		if (newname.empty()) {
			continue;
		}

		// 入力結果の拡張子が".FDX"でなかったらやり直し
		std::wstring wnewname = WStrUtil::str2wstr(newname);
		size_t len = wnewname.length();
		if ((len < 5) || (WStrUtil::wstricmp(&wnewname[len-4], L".FDX"))) {
			FDS_ERROR("cmdRename: Use Extention \".FDX\"!\n");
			FDS_ERROR(" newname=[%s]\n", newname.c_str());
			DlgSelect dlg3;
			dlg3.setItemsOk();
			dlg3.setHeader("Use Extention \".FDX\"!");
			dlg3.setCanEscape(true);
			dlg3.start();
			refreshAllView();
			newfile = newname;
			continue;
		}

		// コピー
		newfile = dlg.getText();
		std::string src = mRootDir + mCurDir + filename;
		std::string dst = mRootDir + mCurDir + newfile;
#if defined(FDS_WINDOWS)
		int ret2 = (CopyFileA(src.c_str(), dst.c_str(), FALSE) ? 0 : -1);
#else
		std::string cmd = "cp";
		std::string option = "-pf \"" + src + "\" \"" + dst + "\"";
		int ret2 = mFddEmu.execCmd(cmd.c_str(), option.c_str());
#endif  // defined(FDS_WINDOW)
		if (ret2 != 0) {
			// 失敗
			FDS_ERROR("cmdDupDisk: Copy Failed!\n");
			FDS_ERROR(" src=[%s], dst=[%s], result=%d\n", src.c_str(), dst.c_str(), ret2);
			DlgSelect dlg3;
			dlg3.setItemsOk();
			dlg3.setHeader("Copy Failed!");
			dlg3.setCanEscape(true);
			dlg3.start();
		}

		// 新しいファイルリストを取得
		mFiles.getFiles(mCurDir.empty());
		mFiles.sortFiles();
		filerViewFindEntry(filename);

		break;
	}
}

// -------------------------------------------------------------
// ディレクトリを作成する
// -------------------------------------------------------------
void
FDSSystem::cmdMakeDirectory()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	std::string filename = "";
	while (!0) {
		// 新しいファイル名を入力
		DlgInput dlg;
		dlg.setHeader("Make Directory:");
		dlg.setText(filename);
		dlg.setCanEscape(true);
		dlg.setMaxLength(FDX_FILENAME_MAX);
		int ret = dlg.start(COLS/2);

		// 入力が行われなかったら中止
		if (ret < 0) {
			return;
		}

		// 入力が空だったら中止
		std::string newname = dlg.getText();
		if (newname.empty()) {
			return;
		}

		// ファイル名に"/"が含まれていたらやり直し
		if (std::string::npos != newname.find('/')) {
			FDS_ERROR("cmdMakeDirectory: Can't use '/'!\n");
			FDS_ERROR(" newname=[%s]\n", newname.c_str());
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Can't use '/'!");
			dlg2.setCanEscape(true);
			dlg2.start();
			refreshAllView();
			filename = newname;
			continue;
		}

		// ディレクトリ作成
		std::string dst = mRootDir + mCurDir + newname;
#if defined(FDS_WINDOWS)
		int ret2 = _mkdir(dst.c_str());
#else
		int ret2 = mkdir(dst.c_str(), 0755);
#endif
		if (ret2 != 0) {
			FDS_ERROR("cmdMakeDirectory: Make Directory Failed!\n");
			FDS_ERROR(" dst=[%s], result=%d, errno=%d\n", dst.c_str(), ret2, errno);
			DlgSelect dlg3;
			dlg3.setItemsOk();
			dlg3.setHeader("Make Directory Failed!");
			dlg3.setCanEscape(true);
			dlg3.start();
		}

		// 新しいファイルリストを取得
		mFiles.getFiles(mCurDir.empty());
		mFiles.sortFiles();
		filerViewFindEntry(newname);

		// 新しいディレクトリの情報を表示
		infoViewSetFile(dst);
		infoViewRefresh();

		return;
	}
}

// -------------------------------------------------------------
// 指定ドライブのイメージを排出する
// -------------------------------------------------------------
void
FDSSystem::cmdEjectDrive()
{
	// 排出先ドライブを選択
	int sel = 0;
	DlgSelect::ItemsVec items;
	for (int i=0; i<mFddEmu.Drives; i++) {
		items.push_back(getDriveName(i));
	}
	{
		DlgSelect dlg;
		dlg.setItemsVec(items);
		dlg.setHeader("[Eject Drive]");
		dlg.setCanEscape(true);
		sel = dlg.start(sel);
		if (sel < 0) {
			return;
		}
	}

	// イメージをドライブへ挿入
	mFddEmu.ejectDrive(sel);

	// FDDビューを更新
	fddViewRefresh();
}

// -------------------------------------------------------------
// ディスク名を編集する
// -------------------------------------------------------------
void
FDSSystem::cmdEditName()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// FDXファイルのみ受け付ける
	if (!mFiles[idx].isFdxFile()) {
		return;
	}

	// 書き込み不可ならエラー
	if (mFiles[idx].isProtect()) {
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("Write Protected!");
		dlg.setCanEscape(true);
		dlg.start();
		return;
	}

	// 選択したFDXファイルで情報ビューを更新
	std::string path = mRootDir + mCurDir + mFiles[idx].filename();
	if (mFiles[idx].isDir()) {
		path += "/";
	}
	infoViewSetFile(path);
	infoViewRefresh();
	int ret = infoViewGetResult();
	if (ret == -1) {
		return;
	}
	if (ret < -1) {
		// 正常に情報取得できなければ中止
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("FDX File Info Error!");
		dlg.setCanEscape(true);
		dlg.start();
		return;
	}

	// ディスク名をダイアログで処理
	FdxHeader& disk = infoViewGetFdxHeader();
	std::string name = std::string((const char*)&(disk.mName[0]));
	{
		// 新しいディスク名を入力
		DlgInput dlg;
		dlg.setHeader("Disk Name:");
		dlg.setMaxLength(59);
		dlg.setText(name.c_str());
		dlg.setCanEscape(true);
		ret = dlg.start(COLS/2);

		// 入力が行われなかったら中止
		if (ret < 0) {
			return;
		}

		// 入力が行われたら変更作業（空もあり）
		std::string newname = dlg.getText();
		std::wstring wnewname = WStrUtil::str2wstr(newname);
		name = WStrUtil::wstr2strN(wnewname, 60);
		strcpy((char *)&(disk.mName[0]), name.c_str());

		// 情報を更新
		int ret2 = infoViewSaveFile();
		if (ret2 < 0) {
			// 正常に更新できなければエラー表示
			FDS_ERROR("cmdEditName: Save Error!\n");
			FDS_ERROR(" path=[%s], name=[%s], result=%d, errno=%d\n", path.c_str(), name.c_str(), ret2, errno);
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Save Error!");
			ret = dlg2.start();
			return;
		}

		// 名前が切り詰められたら報告
		infoViewRefresh();
		if (name != newname) {
			FDS_ERROR("cmdEditName: Disk Name Trancated!\n");
			FDS_ERROR(" path=[%s], before=[%s], after=[%s]\n", path.c_str(), newname.c_str(), name.c_str());
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Disk Name Trancated!");
			dlg2.setCanEscape(true);
			dlg2.start();
		}

		// 新しいファイルリストを取得
		mFiles.getFiles(mCurDir.empty());
		mFiles.sortFiles();
	}
}

// -------------------------------------------------------------
// ドライブのプロテクト状態を変更する
// -------------------------------------------------------------
void
FDSSystem::cmdProtectDrive()
{
	// 排出先ドライブを選択
	int sel = 0;
	DlgSelect::ItemsVec items;
	for (int i=0; i<mFddEmu.Drives; i++) {
		items.push_back(getDriveName(i));
	}
	{
		DlgSelect dlg;
		dlg.setItemsVec(items);
		dlg.setHeader("[Protect Drive]");
		dlg.setCanEscape(true);
		sel = dlg.start(sel);
		if (sel < 0) {
			return;
		}
	}

	// ドライブのプロテクトスイッチを設定
	mFddEmu.protectDrive(sel);

	// FDDビューを更新
	fddViewRefresh();
}

// -------------------------------------------------------------
// ディスクのプロテクト状態を変更する
// -------------------------------------------------------------
void
FDSSystem::cmdEditProtect()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// FDXファイルのみ受け付ける
	if (!mFiles[idx].isFdxFile()) {
		return;
	}

	// 書き込み不可ならエラー
	if (mFiles[idx].isProtect()) {
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("Write Protected!");
		dlg.setCanEscape(true);
		dlg.start();
		return;
	}

	// 選択したエントリで情報ビューを更新
	std::string path = mRootDir + mCurDir + mFiles[idx].filename();
	if (mFiles[idx].isDir()) {
		path += "/";
	}
	infoViewSetFile(path);
	infoViewRefresh();
	int ret = infoViewGetResult();
	if (ret == -1) {
		return;
	}
	if (ret < -1) {
		// 正常に情報取得できなければ中止
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("FDX File Info Error");
		dlg.setCanEscape(true);
		dlg.start();
		return;
	}

	// プロテクトの状態をダイアログで処理
	FdxHeader& disk = infoViewGetFdxHeader();
	bool protect = (disk.mWriteProtect != 0);
	{
		int sel = (protect ? 0 : 1);
		DlgSelect::ItemsArray<3> items {{ "On", "Off", "[ Cancel ]", }};
		DlgSelect dlg;
		dlg.setItemsArray(items);
		dlg.setHeader("[Edit Protect]");
		dlg.setCanEscape(true);
		sel = dlg.start(sel);
		if ((sel == 2) || (sel == -1)) {
			// キャンセルなら中止
			return;
		}
		disk.mWriteProtect = (sel == 0);
	}

	// 情報を更新
	ret = infoViewSaveFile();
	if (ret < 0) {
		// 正常に更新できなければエラー表示
		FDS_ERROR("cmdEditProtect: Save Error!\n");
		FDS_ERROR(" path=[%s], ret=%d, errno=%d\n", path.c_str(), ret, errno);
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("Save Error!");
		dlg.setCanEscape(true);
		ret = dlg.start();
	}

	infoViewRefresh();
}

// -------------------------------------------------------------
// ディレクトリまたはディスクを削除
// -------------------------------------------------------------
void
FDSSystem::cmdDelete()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// ディレクトリかFDXファイルのみ受け付ける
	if (!mFiles[idx].isFdxFile() && !mFiles[idx].isNormalDir()) {
		return;
	}

	// 選択したエントリで情報ビューを更新
	std::string path = mRootDir + mCurDir + mFiles[idx].filename();
	if (mFiles[idx].isDir()) {
		path += "/";
	}
	infoViewSetFile(path);
	infoViewRefresh();
	int ret = infoViewGetResult();
	if (ret == -1) {
		return;
	}
	if (ret < -1) {
		// 正常に情報取得できなければ中止
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("FDX File Info Error");
		dlg.setCanEscape(true);
		dlg.start();
		return;
	}

	{
		// "Y"を入力
		DlgInput dlg;
		dlg.setHeader("Input 'Y' to Delete:");
		dlg.setMaxLength(1);
		dlg.setCanEscape(true);
		ret = dlg.start();

		// 入力が行われなかったら中止
		if (ret < 0) {
			return;
		}

		// 入力が'Y"でなかったら中止
		std::string yes = dlg.getText();
		if (toupper(yes[0]) != 'Y') {
			return;
		}

		// 削除
		std::string src = mRootDir + mCurDir + mFiles[idx].filename();
		int ret2;
		if (mFiles[idx].isDir()) {
#if defined(FDS_WINDOWS)
			ret2 = _rmdir(src.c_str());
#else
			ret2 = rmdir(src.c_str());
#endif  // defined(FDS_WINDOWS)
		} else {
			ret2 = remove(src.c_str());
		}
		if (ret2 != 0) {
			FDS_ERROR("cmdDelete: Delete Failed!\n");
			FDS_ERROR(" src=[%s] result=%d, errno=%d\n", src.c_str(), ret2, errno);
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Delete Failed!");
			dlg2.setCanEscape(true);
			dlg2.start();
			return;
		}

		// 新しいファイルリストを取得
		mFiles.getFiles(mCurDir.empty());
		mFiles.sortFiles();
		filerViewSetIdx(idx);
	}
}

// -------------------------------------------------------------
// シェルを起動
// -------------------------------------------------------------
void
FDSSystem::cmdShell()
{
#if !defined(FDS_WINDOWS)

	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	std::string filename = mFiles[idx].filename();

	// シェルを起動
	move(LINES-1, 0);
	refresh();
	def_prog_mode();
	curs_set(1);
	echo();
	cbreak();
	reset_shell_mode();
	printf("\n");

	// シェル起動
	char* cwd = getcwd(nullptr, 0);
	std::string path = mRootDir + mCurDir;
	chdir(path.c_str());
	char* env = getenv("SHELL");
	system(env);
	chdir(cwd);
	free(cwd);

	// FDSの画面を復帰
	def_shell_mode();
	cbreak();
	noecho();
	curs_set(0);
	reset_prog_mode();
	drawHeader();
	pathViewRedraw();
	infoViewRedraw();
	fddViewRedraw();
	helpViewRedraw();

	// 新しいファイルリストを取得
	mFiles.getFiles(mCurDir.empty());
	mFiles.sortFiles();
	filerViewSetIdx(idx);

#endif
}

// -------------------------------------------------------------
// ファイルの書き込み属性を変更する
// -------------------------------------------------------------
void
FDSSystem::cmdProtectDisk()
{
	// 選択位置をチェック
	int idx = filerViewGetIdx();
	if (idx >= (int)mFiles.size()) {
		return;
	}

	// FDXファイルのみ受け付ける
	if (!mFiles[idx].isFdxFile()) {
		return;
	}

	// 選択したエントリで情報ビューを更新
	std::string path = mRootDir + mCurDir + mFiles[idx].filename();
	if (mFiles[idx].isDir()) {
		path += "/";
	}
	infoViewSetFile(path);
	infoViewRefresh();
	int ret = infoViewGetResult();
	if (ret == -1) {
		return;
	}
	if (ret < -1) {
		// 正常に情報取得できなければ中止
		DlgSelect dlg;
		dlg.setItemsOk();
		dlg.setHeader("FDX File Info Error");
		dlg.setCanEscape(true);
		dlg.start();
		return;
	}

	// プロテクトの状態をダイアログで処理
	bool protect = mFiles[idx].isProtect();
	{
		int sel = (protect ? 0 : 1);
		DlgSelect::ItemsArray<3> items {{ "On", "Off", "[ Cancel ]", }};
		DlgSelect dlg;
		dlg.setItemsArray(items);
		dlg.setHeader("[Protect Disk]");
		dlg.setCanEscape(true);
		sel = dlg.start(sel);
		if ((sel == 2) || (sel == -1)) {
			// キャンセルなら中止
			return;
		}
#if defined(FDS_WINDOWS)
		int ret2 = _chmod(path.c_str(), ((sel == 0) ? _S_IREAD : _S_IREAD| _S_IWRITE));
#else
		int ret2 = chmod(path.c_str(), ((sel == 0) ? 0444 : 0666));
#endif  // defined(FDS_WINDOWS)
		if (ret2 != 0) {
			FDS_ERROR("cmdProtectDisk: Chmod Failed!\n");
			FDS_ERROR(" src=[%s] result=%d, errno=%d\n", path.c_str(), ret2, errno);
			DlgSelect dlg2;
			dlg2.setItemsOk();
			dlg2.setHeader("Chmod Failed!");
			dlg2.setCanEscape(true);
			dlg2.start();
			return;
		}
	}

	// 新しいファイルリストを取得
	mFiles.getFiles(mCurDir.empty());
	mFiles.sortFiles();
}


// =====================================================================
// [EOF]
