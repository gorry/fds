// ◇
// fds: fdx68 selector
// IniFile: INIファイル読み込み
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "IniFile.h"

// =====================================================================
// INIファイル読み込み
// =====================================================================

/*
■ルール

・"#"で始まる行は、コメントとして行末まで読み飛ばす。
・"["で始まる行は、"]"までの文字列を「セクション名」とする。
・それ以外で始まる行は、"="の左側文字列を「キー名」、右側文字列を
  「バリュー」としてデータベースへ登録する。
・行末は0x0d・0x0aどちらでもよい。また0x1aは改行文字と同じ扱いをする。
・データベースは「セクション名」と「キー名」を指定して、データを読み出す。

ファイルの実例は以下の通り。
-------------------------------------------------------------
# コメント

[セクション名]
キー名=バリュー
-------------------------------------------------------------

*/

// -------------------------------------------------------------
// INIファイル読み込み
// -------------------------------------------------------------
int
IniFile::load(const char* filename)
{
	std::string section;
	std::string key;
	std::string value;

	// INIファイルを開く
	FILE *fin = fopen(filename, "r");
	if (fin == nullptr) {
		return -1;
	}

	// ファイル末尾まで
	while (!feof(fin)) {
		// １行読む
		char buf[1024];
		if (!fgets(buf, sizeof(buf), fin)) {
			break;
		}

		char* p = &buf[0];
		char c;
		int mode = 0;
		while ((c = *(p++)) != '\0') {
			switch (mode) {
			  default:
			  case 0:
				// モード0: 行頭読み取り
				if ((c == 0x0d) || (c == 0x0a) || (c == 0x1a)) {
					// 改行またはEOFなら読み飛ばす
					continue;
				}
				if ((c == ' ') || (c == '\t')) {
					// スペース・タブなら読み飛ばす
					continue;
				}
				if (c == '#') {
					// "#"ならコメント扱い、行末まで読み飛ばす
					goto newline;
				}
				if (c == '[') {
					// "["ならセクション名としてモード1へ遷移
					section.clear();
					mode = 1;
					continue;
				}
				// それ以外ならキー名として追加し、モード2へ遷移
				key.clear();
				key.push_back(c);
				mode = 2;
				continue;

			  case 1:
				// モード1: セクション名読み取り
				if ((c == 0x0d) || (c == 0x0a) || (c == 0x1a)) {
					// 改行またはEOFならセクション名終了、モード0へ遷移
					mode = 0;
					continue;
				}
				if (c == ']') {
					// 改行またはEOFならセクション名終了、行末まで読み飛ばす
					goto newline;
				}
				// それ以外ならセクション名として追加
				section.push_back(c);
				continue;

			  case 2:
				// モード2: キー名読み取り
				if ((c == 0x0d) || (c == 0x0a) || (c == 0x1a)) {
					// 改行またはEOFならセクション名終了、モード0へ遷移
					mode = 0;
					continue;
				}
				if (c == '=') {
					// "="ならモード3へ遷移
					value.clear();
					mode = 3;
					continue;
				}
				// それ以外ならキー名として追加
				key.push_back(c);
				continue;

			  case 3:
				// モード3: バリュー読み取り
				if ((c == 0x0d) || (c == 0x0a) || (c == 0x1a)) {
					// 改行またはEOFならバリュー終了、
					// キー/バリューをデータベースへ登録後、モード0へ遷移
					std::string sectionkey = (section.empty() ? key : (section + ":" + key));
					mIniFileMap.insert(std::make_pair(sectionkey, value));
					continue;
				}
				// それ以外ならバリューとして追加
				value.push_back(c);
				continue;

			  case 4:
				// モード4: 行末まで読み飛ばし
				if ((c == 0x0d) || (c == 0x0a) || (c == 0x1a)) {
					// 改行またはEOFなら読み飛ばし終了、モード0へ遷移
					mode = 0;
				}
				continue;

			  newline:
				// 行末まで読み飛ばすモードへ遷移
				mode = 4;
				continue;
			}
		}
	}

	// INIファイルを閉じる
	fclose(fin);
	return 0;
}

// -------------------------------------------------------------
// データベースにキーがあるかどうか調べる
// -------------------------------------------------------------
bool
IniFile::hasKey(const std::string& section, const std::string& key)
{
	std::string sectionkey = (section.empty() ? key : (section + ":" + key));
	if (mIniFileMap.end() == mIniFileMap.find(sectionkey)) {
		return false;
	}
	return true;
}

// -------------------------------------------------------------
// データベースから文字列を読み出す
// -------------------------------------------------------------
const std::string
IniFile::getString(const std::string& section, const std::string& key, const std::string& defaultparam)
{
	std::string sectionkey = (section.empty() ? key : (section + ":" + key));
	if (mIniFileMap.end() == mIniFileMap.find(sectionkey)) {
		return defaultparam;
	}
	return mIniFileMap[sectionkey];
}

// -------------------------------------------------------------
// データベースからint値（10進数）を読み出す
// -------------------------------------------------------------
int
IniFile::getInt(const std::string& section, const std::string& key, int defaultparam)
{
	std::string sectionkey = (section.empty() ? key : (section + ":" + key));
	if (mIniFileMap.end() == mIniFileMap.find(sectionkey)) {
		return defaultparam;
	}
	return atoi(mIniFileMap[sectionkey].c_str());
}

// -------------------------------------------------------------
// INIファイル書き込み
// -------------------------------------------------------------
int
IniFile::save(const char* filename)
{
	std::string lastsection;
	bool isfirstline = true;

	// INIファイルを開く
	FILE *fout = fopen(filename, "w");
	if (fout == nullptr) {
		return -1;
	}

	// マップ内全部（mIniFileMapはsectionkeyでソート済）
	for (const auto& e: mIniFileMap) {
		const std::string& sectionkey = e.first;
		const std::string& value = e.second;
		std::string section;
		std::string key;

		// セクション名とキー名を取り出す
		size_t pos = sectionkey.find(":", 0);
		if (std::string::npos == pos) {
			// ":"で区切られていなければキー名のみ
			key = sectionkey;
		} else {
			// ":"で区切られていればセクション名とキー名を取り出し
			section = sectionkey.substr(0, pos);
			key = sectionkey.substr(pos+1);
		}

		// セクション名が変わる場合はそれを出力
		if (lastsection != section) {
			if (!isfirstline) {
				// 最初のセクションでなければ
				fputs("\n", fout);
			}
			fprintf(fout, "[%s]\n", section.c_str());
			lastsection = section;
		}

		// [key]=[value]を出力
		fprintf(fout, "%s=%s\n", key.c_str(), value.c_str());
		isfirstline = false;
	}

	// INIファイルを閉じる
	fclose(fout);
	return 0;
}

// -------------------------------------------------------------
// データベースへ文字列を登録する
// -------------------------------------------------------------
void
IniFile::setString(const std::string& section, const std::string& key, const std::string& value)
{
	std::string sectionkey = (section.empty() ? key : (section + ":" + key));
	auto e = mIniFileMap.find(sectionkey);
	if (mIniFileMap.end() != e) {
		mIniFileMap.erase(e);
	}
	mIniFileMap.emplace(sectionkey, value);
}

// -------------------------------------------------------------
// データベースへint値（10進数）を登録する
// -------------------------------------------------------------
void
IniFile::setInt(const std::string& section, const std::string& key, int value)
{
	char buf[16];
	sprintf(buf, "%d", value);
	std::string valuestr(buf);
	setString(section, key, valuestr);
}

// =====================================================================
// [EOF]
