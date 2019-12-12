ALL:target

TOPDIR=$(shell pwd);
#INCLUDEDIR=../../
#LIBPATH=../lib/

#becare:受制于C的单遍编译模型，为了减少错误，该项必须位于最后
LINKARCHIVE=-ldigital

CC=gcc
XX=g++
DEBUG=-ggdb -O0
FLAG=-std=c++11 -Wall -I$(INCLUDEDIR) -L$(LIBPATH)  
#用于确定当前需要编译问题的编号，格式必须是凑成三位数，形如NO=012
NO=-1

#REALEASE=0
#设置发行版本，RELEASE为0时(默认)发布debug
ifeq ($(RELEASE),)
	RELEASE=0
endif

ifeq ($(BINPATH),)
	BINPATH=../bin
endif

ifeq ($(INCLUDEDIR),)
	INCLUDEDIR=../../
endif

ifeq ($(LIBPATH),)
	LIBPATH=../lib/
endif

ifeq ($(RELEASE),0)
	FLAG+=$(DEBUG) 
else
	FLAG+=-O3 -static -DNDEBUG
endif

ifeq ($(NO),-1)
	$(error please specify the Number of question,eg."NO=001")
else
	FLAG+=-DNO_$(NO)
endif

solve_main:solve_main.cc
	$(XX) $(FLAG) -o solve_main.ef $^ $(LINKARCHIVE)
	
.PHONY :clean

clean:
	@rm -f *.o *.ef
	@echo Done!


