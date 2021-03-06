PHONY := __build
__build:

obj-y :=
subdir-y :=

include Makefile

# get all subdir name
__subdir-y	:= $(patsubst %/,%,$(filter %/, $(obj-y)))
subdir-y	+= $(__subdir-y)

# get all subdir's built-in.o  c/built-in.o d/built-in.o
subdir_objs := $(foreach f,$(subdir-y),$(f)/built-in.o)

# get all obj files
cur_objs := $(filter-out %/, $(obj-y))

# get all depend files
dep_files := $(foreach f,$(cur_objs),.$(f).d)
dep_files := $(wildcard $(dep_files))

# include all deoend files
ifneq ($(dep_files),)
  include $(dep_files)
endif

PHONY += $(subdir-y)

__build : $(subdir-y) built-in.o

# recursive subdir, than execute common.mk
$(subdir-y):
	make -C $@ -f $(TOPDIR)/common.mk

# link & genarate current dir's built-in.o
built-in.o : $(cur_objs) $(subdir_objs)
	@echo LD $^
	@$(LD) -r -o $@ $^

dep_file = .$@.d

# compile every *.c files
%.o : %.c
	@echo CC $<
	@$(CC) $(CFLAGS) -Wp,-MD,$(dep_file) -c -o $@ $<

.PHONY : $(PHONY)
