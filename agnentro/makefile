BASICFLAGS=-D_FILE_OFFSET_BITS=64 -fno-stack-protector -mtune=native
BITS=32
CC=cc
DELTEMP=rm tmp$(SLASH)*
EXE=
NASM=
OBJ=.o
OPTIMIZEFLAGS=-O3
SLASH=/
TARGET=$(shell cc -dumpmachine)
WARNFLAGS=-Wall -Wextra -Wconversion
ifneq (,$(findstring 64,$(TARGET)))
	BITS=64
	WARNFLAGS+= -Wint-conversion
endif
ifneq (,$(findstring mingw,$(TARGET)))
ifneq (,$(findstring mingw32,$(TARGET)))
	BITS=32
endif
	DELTEMP=@echo clean does not work due to idiotic problems with MAKE under MSYS, but deleting everything in the tmp subfolder accomplishes the same thing.
	EXE=.exe
	OBJ=.obj
	SLASH=\\
endif
ifneq (,$(findstring darwin,$(TARGET)))
	WARNFLAGS+= -Wconstant-conversion
else
ifneq (,$(findstring arm,$(TARGET)))
	WARNFLAGS+= -Wconstant-conversion
endif
endif

agnentrocodec:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)agnentrocodec$(OBJ) agnentrocodec.c

agnentrocodec_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)agnentrocodec$(OBJ) agnentrocodec.c

agnentrofile:
	make ascii
	make agnentrocodec
	make agnentroprox
	make biguint
	make bitscan
	make debug
	make filesys
	make fracterval_u128
	make fracterval_u64
	make loggamma
	make poissocache
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)agnentrocodec$(OBJ) tmp$(SLASH)agnentroprox$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)biguint$(OBJ) tmp$(SLASH)bitscan$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)fracterval_u128$(OBJ) tmp$(SLASH)fracterval_u64$(OBJ) tmp$(SLASH)loggamma$(OBJ) tmp$(SLASH)poissocache$(OBJ) -otmp$(SLASH)agnentrofile$(EXE) agnentrofile.c
	@echo You can now run \"tmp$(SLASH)agnentrofile\".

agnentrofind:
	make ascii
	make agnentroprox
	make biguint
	make bitscan
	make debug
	make filesys
	make fracterval_u128
	make fracterval_u64
	make loggamma
	make poissocache
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)agnentroprox$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)biguint$(OBJ) tmp$(SLASH)bitscan$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)fracterval_u128$(OBJ) tmp$(SLASH)fracterval_u64$(OBJ) tmp$(SLASH)loggamma$(OBJ) tmp$(SLASH)poissocache$(OBJ) -otmp$(SLASH)agnentrofind$(EXE) agnentrofind.c
	@echo You can now run \"tmp$(SLASH)agnentrofind\".

agnentroprox:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)agnentroprox$(OBJ) agnentroprox.c

agnentroprox_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)agnentroprox$(OBJ) agnentroprox.c

agnentroscan:
	make ascii
	make agnentroprox
	make biguint
	make bitscan
	make debug
	make filesys
	make fracterval_u128
	make fracterval_u64
	make loggamma
	make poissocache
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)agnentroprox$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)biguint$(OBJ) tmp$(SLASH)bitscan$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)fracterval_u128$(OBJ) tmp$(SLASH)fracterval_u64$(OBJ) tmp$(SLASH)loggamma$(OBJ) tmp$(SLASH)poissocache$(OBJ) -otmp$(SLASH)agnentroscan$(EXE) agnentroscan.c
	@echo You can now run \"tmp$(SLASH)agnentroscan\".

ascii:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)ascii$(OBJ) ascii.c

ascii_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)ascii$(OBJ) ascii.c

biguint:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)biguint$(OBJ) biguint.c

biguint_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)biguint$(OBJ) biguint.c

bitscan:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)bitscan$(OBJ) bitscan.c

clean:
	$(DELTEMP)

debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)debug$(OBJ) debug.c

filesys:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)filesys$(OBJ) filesys.c

filesys_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)filesys$(OBJ) filesys.c

fracterval_u128:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)fracterval_u128$(OBJ) fracterval_u128.c

fracterval_u128_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)fracterval_u128$(OBJ) fracterval_u128.c

fracterval_u64:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)fracterval_u64$(OBJ) fracterval_u64.c

fracterval_u64_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)fracterval_u64$(OBJ) fracterval_u64.c

loggamma:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)loggamma$(OBJ) loggamma.c

loggamma_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)loggamma$(OBJ) loggamma.c

poissocache:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)poissocache$(OBJ) poissocache.c

poissocache_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)poissocache$(OBJ) poissocache.c

setidemo:
	make ascii
	make agnentroprox
	make biguint
	make bitscan
	make debug
	make filesys
	make fracterval_u128
	make fracterval_u64
	make loggamma
	make poissocache
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)agnentroprox$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)biguint$(OBJ) tmp$(SLASH)bitscan$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)fracterval_u128$(OBJ) tmp$(SLASH)fracterval_u64$(OBJ) tmp$(SLASH)loggamma$(OBJ) tmp$(SLASH)poissocache$(OBJ) -otmp$(SLASH)setidemo$(EXE) setidemo.c
	@echo You can now run \"tmp$(SLASH)setidemo\".
