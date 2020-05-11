// ◇
// fds: fdx68 selector
// main
// Copyright: (C)2020 Hiroaki GOTO as GORRY.
// License: see readme.txt
// =====================================================================

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>

#include "FDSSystem.h"
#include "Log.h"

// =====================================================================
// main
// =====================================================================

// -------------------------------------------------------------
// Win32用setenv
// -------------------------------------------------------------
#if defined(FDS_WINDOWS)
int setenv(const char *name, const char *value, int overwrite)
{
	int err = 0;
	if (!overwrite) {
		size_t size = 0;
		err = getenv_s(&size, NULL, 0, name);
		if (err || size) {
			return err;
		}
	}
	int ret = _putenv_s(name, value);
	return ret;
}
#endif

// -------------------------------------------------------------
// ヘルプ表示
// -------------------------------------------------------------
void showHelp()
{
	printf("%s", 
	   "FDS: FDX68 File Selector: Version " FDS_VERSION " GORRY.\n"
	   "Usage:\n"
	   "  sudo fds      Run FDS with fddemu (root required!)\n"
	   "  fds --noroot  Run FDS only (no root required, run fddemu first!)\n"
	   "Option:\n"
	   "  --log         Output debug log\n"
	   "  --help        Show this\n"
	);
}

// -------------------------------------------------------------
// Ctrl+Cハンドラ
// -------------------------------------------------------------
void abortHandler(int sig)
{
	(void)sig;
}

// -------------------------------------------------------------
// main
// -------------------------------------------------------------
int main(int argc, char **argv)
{
	FDSSystem sys;
	bool log = false;

	for (int i=1; i<argc; i++) {
		std::string arg(argv[i]);
		if (arg == "--help") {
			showHelp();
			return 0;
		}
		if (arg == "--noroot") {
			sys.setNoRoot(true);
		}
		if (arg == "--log") {
			log = true;
		}
	}


#if defined(NDEBUG)
	// Ctrl+Cを殺す
	signal(SIGINT, abortHandler);
#endif  // !defined(NDEBUG)

#if !defined(NDEBUG)
	log = true;
#endif  // !defined(NDEBUG)
	FDSLog::setOutputLog(log);

	// ロケール設定
	char* env = getenv("LANG");
	setlocale(LC_ALL, env);
	FDS_LOG("env: LANG=[%s]\n", env);

	// Curses用にESCキー押下時ディレイ値を設定
	setenv("ESCDELAY", "10", 1);

	// システム開始
	sys.start();
	sys.end();

	printf("\n");

	return 0;
}

// =====================================================================
// [EOF]
