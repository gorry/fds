// ◇
// fds: fdx68 selector
// MakeArgv: Argvの作成
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include "MakeArgv.h"

// -------------------------------------------------------------
// 起動コマンドライン文字列からArgvを生成する
// -------------------------------------------------------------
std::vector<const char*>
makeArgv(const std::string& cmd, const std::string& option, std::vector<std::string>& retArgs)
{
	retArgs.clear();
	retArgs.push_back(cmd.c_str());

	int mode = 0;
	char c = 0;
	char c2 = 0;
	const char* p = option.c_str();
	std::string arg;

	// 文字列終了までループ
	while ((c = *(p++)) != '\0') {
		switch (mode) {
		  default:
		  case 0:  // １文字め
			if ((c == ' ') || (c == '\t')) {
				// スペースとタブは１文字めにしない
				continue;
			}
			if (c == '\\') {
				// "\"で始まるなら次の文字を１文字めとして積む
				c = *(p++);
				if (c == '\0') goto finish;
				arg.push_back(c);
				mode = 1;
				continue;
			}
			if ((c == '\'') || (c == '\"')) {
				// "/'で始まるなら括り開始
				// arg.push_back(c);
				c2 = c;
				mode = 2;
				continue;
			}
			// それ以外ならそのまま１文字めとして積む
			arg.push_back(c);
			mode = 1;
			continue;

		  case 1:  // ２文字め以降
			if ((c == ' ') || (c == '\t')) {
				// スペースかタブなら引数完成
				retArgs.push_back(arg);
				arg.clear();
				mode = 0;
				continue;
			}
			if (c == '\\') {
				// "\"なら次の文字を積む
				c = *(p++);
				if (c == '\0') goto finish;
				arg.push_back(c);
				continue;
			}
			if ((c == '\'') || (c == '\"')) {
				// "/'なら始まるなら括り開始
				c2 = c;
				mode = 2;
				continue;
			}
			// それ以外ならそのまま積む
			arg.push_back(c);
			continue;

		  case 2:  // 括り中
			// 括り開始文字なら括り終了
			if (c == c2) {
				// arg.push_back(c);
				mode = 1;
				continue;
			}
			// それ以外ならそのまま積む
			arg.push_back(c);
			continue;
		}

	  finish:;
		break;
	}

	// 引数として積んだ文字が残っていたらそれを引数に
	if (!arg.empty()) {
		retArgs.push_back(arg);
	}

	// 引数ポインタリストを作成
	std::vector<const char*> argv;
	for (int i=0; i<(int)retArgs.size(); i++) {
		argv.push_back(retArgs[i].c_str());
	}
	argv.push_back(nullptr);

	return argv;
}

// =====================================================================
// [EOF]
