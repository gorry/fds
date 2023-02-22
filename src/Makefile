# gmake
# ◇

FDSARC = fds68k_20230222a.tar.gz

CPPSRCS = \
	main.cpp \
	DirEntry.cpp \
	DirStack.cpp \
	DlgInput.cpp \
	DlgSelect.cpp \
	FddEmu.cpp \
	FdDump.cpp \
	FdRestore.cpp \
	FDSAnalyzer.cpp \
	FDSAnalyzerCmd.cpp \
	FDSAnalyzerDisk.cpp \
	FDSAnalyzerDump.cpp \
	FDSAnalyzerHelp.cpp \
	FDSAnalyzerRestore.cpp \
	FDSAnalyzerSector.cpp \
	FDSAnalyzerTrack.cpp \
	FDSCommon.cpp \
	FDSConfig.cpp \
	FDSDrive.cpp \
	FDSMachine.cpp \
	FDSSystem.cpp \
	FDSSystemCmd.cpp \
	FDSSystemDump.cpp \
	FDSSystemFdd.cpp \
	FDSSystemFiler.cpp \
	FDSSystemHelp.cpp \
	FDSSystemInfo.cpp \
	FDSSystemPath.cpp \
	FDSSystemRestore.cpp \
	FdxConv.cpp \
	FdxStatus.cpp \
	FdxTool.cpp \
	FdxTrkCpy.cpp \
	FdxView.cpp \
	IniFile.cpp \
	Log.cpp \
	MakeArgv.cpp \
	WStrUtil.cpp \
	

CPPOBJS = $(addprefix $(OBJDIR)/,$(CPPSRCS:.cpp=.o))
CPPDEPS = $(addprefix $(OBJDIR)/,$(CPPSRCS:.cpp=.dd))

OUTDIR_DEBUG = Debug
OUTDIR_RELEASE = Release

#CC = clang
#CXX = clang
LD = g++

CXXFLAGS = -c -g -D NCURSES_WIDECHAR=1 -Wall
CXXFLAGS += -Wno-psabi  # to avoid warning > note: parameter passing for argument of type ‘std::move_iterator<>’ changed in GCC 7.1
LDFLAGS = -lncursesw

ifneq ($(NOSUDO),)
CXXFLAGS += -D NOSUDO=1
endif

ifneq ($(DEBUG),)
OUTDIR = $(OUTDIR_DEBUG)
CXXFLAGS +=
LDFLAGS +=
else
OUTDIR = $(OUTDIR_RELEASE)
CXXFLAGS += -O3 -DNDEBUG
LDFLAGS += 
endif

OBJDIR = $(OUTDIR)
EXEDIR = $(OUTDIR)

TARGETNAME = fds

TARGET = $(EXEDIR)/$(TARGETNAME)

.PHONY: target
target: $(TARGET)
$(TARGET): $(CPPOBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(OBJDIR)/%.dd
	$(CXX) $(CXXFLAGS) -o $@ $(patsubst %.dd,%.cpp,$(subst $(OBJDIR)/,,$<))

$(OBJDIR)/%.dd: %.cpp
	mkdir -p $(dir $@)
	echo -n $(OBJDIR)/>$@
	$(CXX) -MM $< >> $@

.PHONY: run
run: target
ifneq ($(NOSUDO),)
	$(TARGET)
else
	sudo $(TARGET)
endif

.PHONY: clean
clean:
	rm -rf $(OUTDIR_DEBUG) $(OUTDIR_RELEASE)

.PHONY: arc
arc:
	rm -rf package
	mkdir -p package/fds/src
	cp -pv *.cpp *.h *.txt *.sln *.vcxproj *.filters Makefile package/fds/src/
	mkdir -p package/fds/src/pdcurses
	cp -pv pdcurses/* package/fds/src/pdcurses/
	cp -pv -r package_ini/* package/fds/
	mkdir -p package/fds/src/wcwidth
	cp -pv wcwidth/*.c wcwidth/*.txt package/fds/src/wcwidth/
	mkdir -p package/fds/src/dirent
	cp -pv dirent/*.c dirent/*.h package/fds/src/dirent/
	mkdir -p package/fds/src/package_ini
	cp -pv -r package_ini/* package/fds/src/package_ini/
	cd package/fds; tar cvzf src.tar.gz src; rm -rf src; cd ../..
	cp -pv Release/fds Release/fds.exe ./
	# strip fds
	cp -pv readme.md fds fds.exe APACHE-LICENSE-2.0.txt readme.txt update.txt package/fds/
	chmod +x package/fds/fds
	cd package; tar cvzf $(FDSARC) fds; cd ..

ifeq ($(MAKECMDGOALS),)
INCLUDE_DEPS = 1
endif
ifeq ($(MAKECMDGOALS),target)
INCLUDE_DEPS = 1
endif
ifeq ($(INCLUDE_DEPS),1)
include $(CPPDEPS)
endif
