ALL:target

TOPDIR=$(shell pwd)

#子目录
SUBDIR=./001-010 ./011-020 ./021-030

export INCLUDEDIR=$(TOPDIR)
export LIBPATH=$(TOPDIR)/lib
export BINPATH=$(TOPDIR)/bin

#本项目的所有静态库打包
LIBNAME=libdigital.a

CC=gcc
XX=g++
DEBUG=-ggdb -O0
FLAG+=-std=c++11 -Wall -I$(INCLUDEDIR)

export RELEASE=0

ifeq ($(RELEASE),0)
	FLAG+=$(DEBUG) 
else
	FLAG+=-O3 -static -DNDEBUG
endif


%.o:%.cc
	$(XX) $(FLAG) -c $< -o $@

SOURCES=$(wildcard *.c *.cc *.cpp)
OBJECTS=$(patsubst %.c,%.o,$(patsubst %.cc,%.o,$(patsubst %.cpp,%.o,$(SOURCES))))

showall:
	@echo $(OBJECTS)
	@echo $(FLAG)
	@echo $(RELEASE)



target:$(OBJECTS) $(SUBDIR) 
	@echo file list:
	@echo $(OBJECTS)
	@mv *.o $(LIBPATH) || echo none obj
	@cd $(LIBPATH) && (ar crs $(LIBNAME) *.o || echo none obj)
	@echo done!

$(SUBDIR):ECHO
	$(MAKE) -C $@

ECHO:
	@echo ...

.PHONY:clean

clean:
	@for dir in $(SUBDIR);\
		do $(MAKE) -C $$dir clean || exit 1;\
		done
	rm -f *.o
	@cd $(LIBPATH) && rm -f *.o *.a
	@cd $(BINPATH) && rm -f *.ef
	@echo $(TOPDIR) object files has been removed!
