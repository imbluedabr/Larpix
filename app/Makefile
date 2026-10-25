
SYSROOT ?= ./sysroot
ARCH ?= armv8-m
INSTALL_DIR = $(SYSROOT)/bin
ETC_INSTALL_DIR = $(SYSROOT)/etc

-include $(SYSROOT)/lib/libc.mk

CXXFLAGS += -Os
LDFLAGS += -Os -lgcc

CC = $(TOOLCHAIN)-gcc
CXXSRCS = $(wildcard *.cpp)
OBJS = $(CXXSRCS:%.cpp=%.o)

%.o: %.cpp
	$(TOOLCHAIN)-g++ $(CXXFLAGS) -c $<

app: $(OBJS)
	$(TOOLCHAIN)-g++ $(OBJS) $(LDFLAGS) -o app.elf
	$(TOOLCHAIN)-objcopy -O binary app.elf app

all: clean install

install: app
	mkdir -p $(INSTALL_DIR)
	mv app $(INSTALL_DIR)/app

clean:
	rm -f *.o *.elf

