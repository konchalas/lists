ROOTDIR ?= ./../
LOCALDIR ?= ./
BINDIR		?= $(ROOTDIR)bin
OBJDIR ?= $(ROOTDIR)build
CC		= gcc
INCLUDES  += -I$(ROOTDIR)include -I$(ROOTDIR) -I$(LOCALDIR)
BINS = $(OBJDIR)/lazy_list
# VERSION = DEBUG
MODE = LI

# Version
CFLAGS += $(INCLUDES)  -pthread -std=gnu11 -D_GNU_SOURCE
ifeq ($(VERSION),DEBUG)
     CFLAGS     += -g -DDEBUG -O0 -fsanitize=address
else
     CFLAGS     += -O3 -DNDEBUG
endif

ifeq ($(MODE),LI)
OBJECTS := \
	$(OBJDIR)/coupling.o	    \
	$(OBJDIR)/intset.o	    \
	$(OBJDIR)/lazy.o	    \
	$(OBJDIR)/linkedlist-lock.o	    \
	$(OBJDIR)/bench_LI.o	    \
	$(OBJDIR)/system.o	    \
  $(OBJDIR)/fastrand.o 	\
  $(OBJDIR)/bench_utils.o 	\
  $(OBJDIR)/test.o
else
OBJECTS := \
	$(OBJDIR)/coupling.o	    \
	$(OBJDIR)/intset.o	    \
	$(OBJDIR)/lazy.o	    \
	$(OBJDIR)/linkedlist-lock.o	    \
	$(OBJDIR)/bench_check.o	    \
	$(OBJDIR)/system.o	    \
  $(OBJDIR)/fastrand.o 	\
  $(OBJDIR)/bench_utils.o 	\
  $(OBJDIR)/test.o
endif

.PHONY:	all clean

all:  $(OBJDIR) $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(BINS) $(LDFLAGS)

$(OBJDIR):
	mkdir $(subst /,\\,$(OBJDIR))

$(OBJDIR)/intset.o: $(LOCALDIR)intset.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/coupling.o: $(LOCALDIR)coupling.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/linkedlist-lock.o: $(LOCALDIR)linkedlist-lock.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/lazy.o: $(LOCALDIR)lazy.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/bench_check.o: $(ROOTDIR)benchmarks/bench_check.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/bench_LI.o: $(ROOTDIR)benchmarks/bench_LI.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/bench_utils.o: $(ROOTDIR)benchmarks/bench_utils.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/epoch.o: $(ROOTDIR)mem_interface/epoch.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/gc.o: $(ROOTDIR)mem_interface/gc.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/mem_interface_common.o: $(ROOTDIR)mem_interface/mem_interface_common.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/mem_interface_pool.o: $(ROOTDIR)mem_interface/mem_interface_pool.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/mem_interface_system.o: $(ROOTDIR)mem_interface/mem_interface_system.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/system.o: $(ROOTDIR)system/system.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/fastrand.o: $(ROOTDIR)system/fastrand.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/test.o: $(LOCALDIR)test.c
	$(CC) $(CFLAGS) -c -o $@ $^


clean:
	-rm -f $(BINS) $(OBJECTS)
