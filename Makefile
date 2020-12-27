
CXX = gcc
LIB =
RM = rm -fr
CFLAGS = -c -O2 -Wall -D_7ZIP_ST -I./liblzma
OUT=out

PROG = $(OUT)/lzma

OBJS = \
  $(OUT)/liblzma/Alloc.o \
  $(OUT)/liblzma/LzFind.o \
  $(OUT)/liblzma/lzmaDec.o \
  $(OUT)/liblzma/lzmaEnc.o \
  $(OUT)/liblzma/7zFile.o \
  $(OUT)/liblzma/7zStream.o \
  $(OUT)/main.o


all: SETUP $(PROG)

SETUP:
	@mkdir -p $(OUT)/liblzma

$(PROG): $(OBJS)
	$(CXX) -o $(PROG) $(LDFLAGS) $(OBJS) $(LIB)

$(OUT)/main.o: main.c
	$(CXX) $(CFLAGS) main.c -o $(OUT)/main.o

$(OUT)/liblzma/Alloc.o: ./liblzma/Alloc.c
	$(CXX) $(CFLAGS) ./liblzma/Alloc.c -o $(OUT)/liblzma/Alloc.o

$(OUT)/liblzma/LzFind.o: ./liblzma/LzFind.c
	$(CXX) $(CFLAGS) ./liblzma/LzFind.c -o $(OUT)/liblzma/LzFind.o

$(OUT)/liblzma/lzmaDec.o: ./liblzma/lzmaDec.c
	$(CXX) $(CFLAGS) ./liblzma/lzmaDec.c -o $(OUT)/liblzma/lzmaDec.o

$(OUT)/liblzma/lzmaEnc.o: ./liblzma/lzmaEnc.c
	$(CXX) $(CFLAGS) ./liblzma/lzmaEnc.c -o $(OUT)/liblzma/lzmaEnc.o

$(OUT)/liblzma/7zFile.o: ./liblzma/7zFile.c
	$(CXX) $(CFLAGS) ./liblzma/7zFile.c -o $(OUT)/liblzma/7zFile.o

$(OUT)/liblzma/7zStream.o: ./liblzma/7zStream.c
	$(CXX) $(CFLAGS) ./liblzma/7zStream.c -o $(OUT)/liblzma/7zStream.o

clean:
	-$(RM) $(PROG) $(OBJS)
	-$(RM) $(OUT)
