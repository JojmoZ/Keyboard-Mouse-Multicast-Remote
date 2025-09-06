define rwildcard
  $(wildcard $1$2) \
  $(foreach d,$(wildcard $1*/),$(call rwildcard,$d,$2))
endef

CC = g++
BUILD_FOLDER = ./build
INCLUDES = -I ./vendor/asio/asio/include
CFLAGS = -Wall -g -std=c++17 -m64
LDFLAGS =   # nothing special needed for sockets/threads

SRCS  = main_server.cpp main_client.cpp test.cpp $(call rwildcard,internal_lib/,*.cpp)
OBJS = $(patsubst %.cpp,$(BUILD_FOLDER)/%.o,$(notdir $(SRCS))) 

SRCSS  = main_server.cpp  $(call rwildcard,internal_lib/,*.cpp)
OBJSS =  $(patsubst %.cpp,$(BUILD_FOLDER)/%.o,$(notdir $(SRCSS))) 

SRCSC = main_client.cpp $(call rwildcard,internal_lib/,*.cpp)
OBJSC =  $(patsubst %.cpp,$(BUILD_FOLDER)/%.o,$(notdir $(SRCSC)))

SRCST = test.cpp $(call rwildcard,internal_lib/,*.cpp)
OBJST =  $(patsubst %.cpp,$(BUILD_FOLDER)/%.o,$(notdir $(SRCST)))

vpath %.cpp $(sort $(dir $(SRCS)))

SUBDIRS := $(wildcard internal_lib/*/)

all: $(BUILD_FOLDER)/main_server $(BUILD_FOLDER)/main_client

test : $(BUILD_FOLDER)/test

$(BUILD_FOLDER)/test: $(OBJST) | $(BUILD_FOLDER)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

$(BUILD_FOLDER)/main_server: $(OBJSS) | $(BUILD_FOLDER)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

$(BUILD_FOLDER)/main_client: $(OBJSC) | $(BUILD_FOLDER)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

$(BUILD_FOLDER)/%.o: %.cpp | $(BUILD_FOLDER)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_FOLDER):
	mkdir -p "$(BUILD_FOLDER)/"

clean:
	rm -rf $(BUILD_FOLDER)
