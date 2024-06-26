# TODO generate mp4v2/libplatform/config.h
# usually this is generated by configure

# prefer CC and CXX from env
#CC ?= clang
#CXX ?= clang++

# force clang
# clang gives better error messages than gcc
#CC = clang
#CXX = clang++

TARGET_EXEC ?= qaac

BUILD_DIR ?= build

# based on vcproject/qaac/qaac.vcxproj
SRCS := \
	input/CoreAudioPacketDecoder.cpp \
	input/ExtAFSource.cpp \
	input/MP4Source.cpp \
	input/MPAHeader.cpp \
	input/InputFactory.cpp \
	filters/CoreAudioResampler.cpp \
	AudioConverterX.cpp \
	AudioConverterXX.cpp \
	cautil.cpp \
	CoreAudioEncoder.cpp \
	CoreAudioPaddedEncoder.cpp \
	lpc.c \
	main.cpp \
	options.cpp \
	version.cpp \

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

DEPS := $(OBJS:.o=.d)

# -I. -Iinput -Ioutput -Imp4v2 -Imp4v2/include -ICoreAudio -Ifilters -Iinclude -Iinclude/opus -Ivcproject/mp4v2/include
INC_DIRS := \
	. \
	input \
	output \
	mp4v2 \
	mp4v2/include \
	CoreAudio \
	filters \
	include \
	include/opus \
	vcproject/mp4v2/include \

INC_FLAGS := $(addprefix -I,$(INC_DIRS))

COMMON_FLAGS = -MMD -MP -Wfatal-errors -Werror -DQAAC=1 -DNO_COREAUDIO=1 -Wno-multichar

CFLAGS ?= $(INC_FLAGS) $(COMMON_FLAGS)

CXXFLAGS ?= $(INC_FLAGS) $(COMMON_FLAGS) -std=gnu++20

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
