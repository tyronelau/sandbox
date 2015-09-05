CFLAGS += -fPIC -O3 -g -DDEBUG -fvisibility=hidden  -fvisibility-inlines-hidden
CXXFLAGS += -fPIC -O3 -g -DDEBUG -std=c++0x -fvisibility=hidden  -fvisibility-inlines-hidden
DLL_LDFLAGS += -shared -fPIC -Wl,-z,defs
EXE_LDFLAGS += -Wl,-z,defs

BUILD_PATH := build
DEP_PATH := $(BUILD_PATH)/.deps
OBJ_PATH := $(BUILD_PATH)/.objs
BIN_PATH := $(BUILD_PATH)/bin
DLL_SOURCES = dll_test.cpp
EXE_SOURCES = main_test.cpp

EXE_LDLIBS = -ldl -L $(BIN_PATH) -limpl

.SECONDEXPANSION:
.PHONY: all pre clean

all: pre $$(DLL_TARGET) $$(EXE_TARGET)

pre:
	@mkdir -p $(BUILD_PATH)
	@mkdir -p $(DEP_PATH)
	@mkdir -p $(OBJ_PATH)
	@mkdir -p $(BIN_PATH)

clean:
	rm -rf $(BIN_PATH)
	rm -rf $(OBJ_PATH)

clean_all:
	rm -rf $(BUILD_PATH)

$(OBJ_PATH)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_PATH)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

DLL_TARGET := $(BIN_PATH)/libimpl.so

$(DLL_TARGET): $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(basename $(DLL_SOURCES))))
	$(CXX) $(CXXFLAGS) -o $@ $^ $(DLL_LDFLAGS) $(EXTRA_LDFLAGS) #  $(DLL_LDLIBS)

EXE_TARGET := $(BIN_PATH)/main_test.exe

$(EXE_TARGET): $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(basename $(EXE_SOURCES)))) $(DLL_TARGET)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(EXE_LDFLAGS) $(EXTRA_LDFLAGS) $(EXE_LDLIBS) #-Wl,-dynamic-linker,/home/tyrone/bin/lib/ld-linux-x86-64.so.2

