CFLAGS += -fPIC -O0 -g -DDEBUG -fvisibility=hidden  -fvisibility-inlines-hidden
CXXFLAGS += -fPIC -O0 -g -DDEBUG -std=c++0x -fvisibility=hidden  -fvisibility-inlines-hidden
LDFLAGS += -shared -fPIC -ldl -lunwind -lpthread -Wl,-z,defs -Wl,--exclude-libs,ALL

BUILD_PATH := build
DEP_PATH := $(BUILD_PATH)/.deps
OBJ_PATH := $(BUILD_PATH)/.objs
BIN_PATH := $(BUILD_PATH)/lib
SOURCES = backtrace.cpp dllmain.cpp CityHash.cpp guard_thread.cpp

.SECONDEXPANSION:
.PHONY: all pre clean

all: pre $$(REAL_TARGET)

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

REAL_TARGET := $(BIN_PATH)/libdebug.so

$(REAL_TARGET): $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(basename $(SOURCES))))
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(EXTRA_LDFLAGS) $(LDLIBS) 

