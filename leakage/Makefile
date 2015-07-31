CFLAGS += -fPIC -O0 -g -P -DDEBUG -fvisibility=hidden  -fvisibility-inlines-hidden
INCLUDE_DIR = -Icontainer
CXXFLAGS += -fPIC -O0 -g -DDEBUG -std=c++0x -fvisibility=hidden  -fvisibility-inlines-hidden -fno-exceptions
LDFLAGS += -shared -fPIC -ldl -lunwind -lpthread -lc -lm -Wl,-z,defs -Wl,--exclude-libs,ALL -lgcc #-nodefaultlibs

BUILD_PATH := build
DEP_PATH := $(BUILD_PATH)/.deps
OBJ_PATH := $(BUILD_PATH)/.objs
BIN_PATH := $(BUILD_PATH)/lib
SOURCES = backtrace.cpp dllmain.cpp CityHash.cpp guard_thread.cpp callstack_linux.cpp \
					container/hashtable-aux.cpp malloc/malloc.c malloc/itoa.c

.SECONDEXPANSION:
.PHONY: all pre clean

all: pre $$(REAL_TARGET)

pre:
	@mkdir -p $(BUILD_PATH)
	@mkdir -p $(DEP_PATH)
	@mkdir -p $(OBJ_PATH)
	@mkdir -p $(OBJ_PATH)/container
	@mkdir -p $(OBJ_PATH)/malloc
	@mkdir -p $(BIN_PATH)

clean:
	rm -rf $(BIN_PATH)
	rm -rf $(OBJ_PATH)

clean_all:
	rm -rf $(BUILD_PATH)

$(OBJ_PATH)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_PATH)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIR) -o $@ -c $<

REAL_TARGET := $(BIN_PATH)/libdebug.so

LINKER = $(CXX)
$(REAL_TARGET): $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(basename $(SOURCES))))
	$(LINKER) -o $@ $^ $(LDFLAGS) $(EXTRA_LDFLAGS) #-###

