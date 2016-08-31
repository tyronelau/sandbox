.SECONDEXPANSION:

VERSION = 1.6

CC            = gcc
CXX           = g++
RELEASE_CFLAGS        = -pipe -O3 -fPIC -Wall -W -DNEDBUG=1 -g -fvisibility=hidden -Wconversion
RELEASE_CXXFLAGS      = -pipe -std=c++0x -fPIC -g -fno-omit-frame-pointer -DNDEBUG=1 -Wconversion \
												-DGIT_DESC=\"`git rev-parse HEAD`\" -O3 \
												-Wall -W  $(SDK_FLAGS) $(DEFINES) -fvisibility=hidden

DEBUG_CFLAGS        = -pipe -O0 -Wall -W -fPIC -g -fvisibility=hidden -Wconversion # -fsanitize=address -Wconversion
DEBUG_CXXFLAGS      = -pipe -std=c++0x -fPIC -g -fno-omit-frame-pointer -Wconversion \
											-DGIT_DESC=\"`git rev-parse HEAD`\" -O0 \
											-Wall -W  $(SDK_FLAGS) $(DEFINES) -fvisibility=hidden # -fsanitize=address

INCPATH       =  -Iinternal -I.
LINK          = g++
# LDFLAGS        = -static-libgcc -static-libstdc++  # -Wl,-Bstatic,-lc -Wl,-Bstatic,-lm -Wl,-Bdynamic

DEBUG_LINK_LIBS = -levent -levent_thread # -lasan 

AR            = ar cqs

OBJ_DIR = .objs
DEP_DIR = .deps
BIN_DIR = bin

BASE_SOURCES = $(wildcard base/*.cpp)
XCODER_SOURCES  = $(wildcard xcoder/*.cpp)

SOURCES = $(BASE_SOURCES) $(XCODER_SOURCES)

-include $(addprefix $(DBG_DEP_PATH)/, $(addsuffix .d, $(SOURCES)))
-include $(addprefix $(RLS_DEP_PATH)/, $(addsuffix .d, $(SOURCES)))

DBG_OBJ_PATH = debug/$(OBJ_DIR)
DBG_DEP_PATH = debug/$(DEP_DIR)
DBG_BIN_PATH = debug/$(BIN_DIR)

DBG_BASE_OBJECTS = $(addprefix $(DBG_OBJ_PATH)/, $(addsuffix .o, $(basename $(BASE_SOURCES))))
DBG_XCODER_OBJECTS = $(addprefix $(DBG_OBJ_PATH)/, $(addsuffix .o, $(basename $(XCODER_SOURCES))))

RLS_OBJ_PATH = release/$(OBJ_DIR)
RLS_DEP_PATH = release/$(DEP_DIR)
RLS_BIN_PATH = release/$(BIN_DIR)

RLS_BASE_OBJECTS = $(addprefix $(RLS_OBJ_PATH)/, $(addsuffix .o, $(basename $(BASE_SOURCES))))
RLS_XCODER_OBJECTS = $(addprefix $(RLS_OBJ_PATH)/, $(addsuffix .o, $(basename $(XCODER_SOURCES))))

.PHONY: all clean debug release pre

all: pre debug release

$(DBG_OBJ_PATH)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(DBG_DEP_PATH)/$<.d)
	$(CXX) -Wp,-MP,-MD,$(DBG_DEP_PATH)/$<.d -c $(DEBUG_CXXFLAGS) $(INCPATH) -o $@ $<
	@sed -e "s/\(.*\.o:\)/$(subst /,\/,$(dir $(DBG_OBJ_PATH)/$<))\1/" $(DBG_DEP_PATH)/$<.d >$(DBG_DEP_PATH)/$<.d.tmp
	@mv $(DBG_DEP_PATH)/$<.d.tmp $(DBG_DEP_PATH)/$<.d

$(DBG_OBJ_PATH)/%.o: %.c
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(DBG_DEP_PATH)/$<.d)
	$(CC) -Wp,-MP,-MD,$(DBG_DEP_PATH)/$<.d -c $(DEBUG_CFLAGS) $(INCPATH) -o "$@" "$<"
	@sed -e "s/\(.*\.o:\)/$(subst /,\/,$(dir $(DBG_OBJ_PATH)/$<))\1/" $(DBG_DEP_PATH)/$<.d >$(DBG_DEP_PATH)/$<.d.tmp
	@mv $(DBG_DEP_PATH)/$<.d.tmp $(DBG_DEP_PATH)/$<.d

$(RLS_OBJ_PATH)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(RLS_DEP_PATH)/$<.d)
	$(CXX) -Wp,-MP,-MD,$(RLS_DEP_PATH)/$<.d -c $(RELEASE_CXXFLAGS) $(INCPATH) -o "$@" "$<"
	@sed -e "s/\(.*\.o:\)/$(subst /,\/,$(dir $(RLS_OBJ_PATH)/$<))\1/" $(RLS_DEP_PATH)/$<.d >$(RLS_DEP_PATH)/$<.d.tmp
	@mv $(RLS_DEP_PATH)/$<.d.tmp $(RLS_DEP_PATH)/$<.d

$(RLS_OBJ_PATH)/%.o: %.c
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(RLS_DEP_PATH)/$<.d)
	$(CC) -Wp,-MP,-MD,$(RLS_DEP_PATH)/$<.d -c $(RELEASE_CFLAGS) $(INCPATH) -o "$@" "$<"
	@sed -e "s/\(.*\.o:\)/$(subst /,\/,$(dir $(RLS_OBJ_PATH)/$<))\1/" $(RLS_DEP_PATH)/$<.d >$(RLS_DEP_PATH)/$<.d.tmp
	@mv $(RLS_DEP_PATH)/$<.d.tmp $(RLS_DEP_PATH)/$<.d

DBG_BASE_TARGET = $(DBG_BIN_PATH)/libbase.a
DBG_XCODER_TARGET = $(DBG_BIN_PATH)/xcoder

RLS_BASE_TARGET = $(RLS_BIN_PATH)/libbase.a
RLS_XCODER_TARGET = $(RLS_BIN_PATH)/xcoder

$(DBG_BASE_TARGET): $(DBG_BASE_OBJECTS)
	@mkdir -p $(dir $@)
	ar cqs "$@" $^

$(DBG_XCODER_TARGET): $(DBG_XCODER_OBJECTS) $(DBG_BASE_TARGET)
	@mkdir -p $(dir $@)
	$(LINK) $(LDFLAGS) -o "$@" $(DBG_XCODER_OBJECTS) \
		-Llibs -L$(DBG_BIN_PATH) -lbase -lagorartc -lHDACEngine -lfdkaac -lsrs_librtmp -pthread -lpthread

$(RLS_BASE_TARGET): $(RLS_BASE_OBJECTS)
	@mkdir -p $(dir $@)
	ar cqs "$@" $^

$(RLS_XCODER_TARGET): $(RLS_XCODER_OBJECTS) $(RLS_BASE_TARGET) $(RLS_STUB_TARGET)
	@mkdir -p $(dir $@)
	$(LINK) $(LDFLAGS) -o "$@" $(RLS_XCODER_OBJECTS) \
		-Llibs -L$(RLS_BIN_PATH) -lbase -lagorartc -lHDACEngine -lfdkaac -lsrs_librtmp -pthread -lpthread

debug: $(DBG_BASE_TARGET) $(DBG_XCODER_TARGET)
	ln -f libs/libHDACEngine.so $(DBG_BIN_PATH)/libHDACEngine.so
	ln -f libs/libagorartc.so $(DBG_BIN_PATH)/libagorartc.so

release: $(RLS_BASE_TARGET) $(RLS_XCODER_TARGET)
	ln -f libs/libHDACEngine.so $(RLS_BIN_PATH)/libHDACEngine.so
	ln -f libs/libagorartc.so $(RLS_BIN_PATH)/libagorartc.so

pre:
	@mkdir -p $(DBG_OBJ_PATH)
	@mkdir -p $(DBG_DEP_PATH)
	@mkdir -p $(DBG_BIN_PATH)
	@mkdir -p $(RLS_OBJ_PATH)
	@mkdir -p $(RLS_DEP_PATH)
	@mkdir -p $(RLS_BIN_PATH)

clean:
	rm -rf $(DBG_OBJ_PATH)
	rm -rf $(DBG_BIN_PATH)
	rm -rf $(RLS_OBJ_PATH)
	rm -rf $(RLS_BIN_PATH)
