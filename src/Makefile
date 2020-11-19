# gmake
# ◇

FDSARC = fds68k_20201104a.tar.gz

CPPSRCS = \
	main.cpp \
	DirEntry.cpp \
	DirStack.cpp \
	DlgInput.cpp \
	DlgSelect.cpp \
	FddEmu.cpp \
	FdDump.cpp \
	FdRestore.cpp \
	FDSSystem.cpp \
	FDSSystemCmd.cpp \
	FDSSystemDump.cpp \
	FDSSystemFdd.cpp \
	FDSSystemFiler.cpp \
	FDSSystemHelp.cpp \
	FDSSystemInfo.cpp \
	FDSSystemPath.cpp \
	FDSSystemRestore.cpp \
	FdxTool.cpp \
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
	cp *.cpp *.h *.txt *.ini *.sln *.vcxproj *.filters Makefile package/fds/src/
	mkdir -p package/fds/src/wcwidth
	cp wcwidth/*.c wcwidth/*.txt package/fds/src/wcwidth/
	mkdir -p package/fds/src/dirent
	cp dirent/*.c dirent/*.h package/fds/src/dirent/
	cd package/fds; tar cvzf src.tar.gz src; rm -rf src; cd ../..
	-cp Release/fds ./
	cp ../readme.md fds APACHE-LICENSE-2.0.txt fds.ini readme.txt update.txt package/fds/
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
