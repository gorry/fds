// ◇
// fds: fdx68 selector
// DirEntry: ディレクトリエントリ
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "DirEntry.h"

// =====================================================================
// ディレクトリエントリ
// =====================================================================

// -------------------------------------------------------------
// コンストラクタ
// -------------------------------------------------------------
DirEntry::DirEntry(const std::string& filename, DirEntry::Type t)
{
	mFileName = filename;
	mType = t;
	mProtect = false;

	mWFileName = WStrUtil::str2wstr(mFileName);

	// OtherFileとして定義しても、拡張子が".FDX"ならFdxFileとして定義される
	size_t len = mWFileName.length();
	if (mType == Type::OtherFile) {
		if (len >= 5) {
			if (!WStrUtil::wstricmp(&mWFileName[len-4], L".FDX")) {
				mType = Type::FdxFile;
			}
		}
	}
}

// =====================================================================
// ディレクトリ
// =====================================================================

// -------------------------------------------------------------
// コンストラクタ
// -------------------------------------------------------------
Directory::Directory(const std::string& path)
 : mPath(path)
{
}

// -------------------------------------------------------------
// pathの設定
// -------------------------------------------------------------
void
Directory::setPath(const std::string& path)
{
	mPath = path;
}

// -------------------------------------------------------------
// FDXファイル以外をマスクする設定
// -------------------------------------------------------------
void
Directory::setMaskFdxFile(bool sw)
{
	mMaskFdxFile = sw;
}

// -------------------------------------------------------------
// pathのディレクトリエントリを取得
// -------------------------------------------------------------
void
Directory::getFiles(bool isRoot)
{
	mFiles.clear();

	DIR* dirp;
	dirp = opendir(mPath.c_str());
	if (dirp) {
		struct dirent* ent;
		while ((ent = readdir(dirp)) != NULL) {
			// "."は常に無視
			if (!strcmp(ent->d_name, ".")) {
				continue;
			}
			// ".."はシステム毎のルートディレクトリでは無視
			if (!strcmp(ent->d_name, "..")) {
				if (!isRoot) {
					if (!mMaskFdxFile) {
						mFiles.push_back(DirEntry(ent->d_name, DirEntry::Type::ParentDir));
					}
				}
				continue;
			}
			if (ent->d_type & DT_DIR) {
				// 通常ディレクトリ
				if (mMaskFdxFile) {
					continue;
				}
				mFiles.push_back(DirEntry(ent->d_name, DirEntry::Type::Dir));
			} else if (ent->d_type & DT_REG) {
				// 通常ファイル（FDXファイルを含む）
				DirEntry entry(ent->d_name, DirEntry::Type::OtherFile);
				if (entry.isFdxFile()) {
					mFiles.push_back(entry);
				} else {
					if (mMaskFdxFile) {
						continue;
					}
					mFiles.push_back(entry);
				}
			}
			std::string path = mPath + std::string(ent->d_name);
#if defined(FDS_WINDOWS)
			struct _stat st;
			_stat(path.c_str(), &st);
			mFiles.back().setProtect((st.st_mode & _S_IWRITE) == 0);
#else
			struct stat st;
			lstat(path.c_str(), &st);
			mFiles.back().setProtect((st.st_mode & S_IWUSR) == 0);
#endif
		}
		closedir(dirp);
	}
	if (mFiles.size() == 0) {
		DirEntry entry("[No File]", DirEntry::Type::Unknown);
		mFiles.push_back(entry);
		
	}
}

// -------------------------------------------------------------
// ディレクトリエントリをソート
// -------------------------------------------------------------
void
Directory::sortFiles()
{
	struct {
		bool operator()(const DirEntry& left, const DirEntry& right) {
			// エントリタイプが同じなら名前の昇順
			if (left.type() == right.type()) {
				return (left.filename() < right.filename());
			}
			// ParentDir/Dir/FdxFile/OtherFileの順にソート
			return (left.type() < right.type());
		}
	} sortOp;
	std::sort(mFiles.begin(), mFiles.end(), sortOp);
}

// =====================================================================
// [EOF]
