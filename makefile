#variables
compiler:=gcc
#directories
srcDirectory:=src
binDirectory:=bin
includeDirectory:=include
archiveDirectory:=archive
#flags
flags:=-x -std=c17 -Wpedantic -Wall -Wextra
flags+= -Wshadow -Wundef -Wstrict-prototypes -Wswitch-default -Wno-unused-result
flags+= -I$(includeDirectory)
libs:=-lm -lpthread -L./libs
#sources
SOURCES:=$(wildcard $(srcDirectory)/*.c)

#name of the executable file
binName:=logicPractical

#rules
#choose this one to debug
dbg:flags+= -g3 -O0 -fsanitize=address -fsanitize=undefined
dbg:$(binDirectory)/$(binName)

#choose this one for the release version
rls:flags+= -g0 -O2
rls:$(binDirectory)/$(binName)

#choose this one to archive these files into a static library
arc:flags+= -g0 -O2
arc:$(archiveDirectory)/lib$(binName).a
arc:
	@rm *.o

#build rules
#reg
$(binDirectory)/$(binName):$(SOURCES)
	@$(compiler) $(SOURCES) $(flags) $(libs) -o $@

#lib
$(archiveDirectory)/lib$(binName).a:*.o
	@ar -cvq $@ *.o

*.o:$(SOURCES)
	@$(compiler) -c $(SOURCES) $(flags) $(libs) 

#generate default folders
genDir:
	@mkdir archive bin in include libs src

#clean binary files
clean:
	@find bin -type f ! -name "*.*" -delete

#clean archive file
cleanArc:
	@find archive -type f -name "*.a" -delete

.PHONY: genDir clean dbg rls arc cleanArc
