#
# create a compressed vmlinuz image from the original vmlinux
#
include $(srctree)/arch/csky/gx3xxx/Makefile.boot
OBJS=misc.o
HEAD=head.o

targets       := vmlinux vmlinux.lds piggy.gz piggy.o \
		 head.o misc.o $(OBJS)
SEDFLAGS        = s/TEXT_START/$(ZTEXTADDR)/;s/BSS_START/$(ZBSSADDR)/

LDFLAGS_vmlinux := --defsym zreladdr=$(IZTEXTADDR)

LDFLAGS_vmlinux +=-Ttext $(OPERADDR) --no-undefined -X \
	$(shell $(CC) $(KBUILD_CFLAGS) --print-libgcc-file-name) -T

$(obj)/vmlinux: $(obj)/vmlinux.lds $(obj)/$(HEAD) $(obj)/piggy.o \
		$(addprefix $(obj)/, $(OBJS)) FORCE
	$(call if_changed,ld)
	@:


$(obj)/piggy.gz: $(obj)/../Image FORCE
	$(call if_changed,gzip)

$(obj)/piggy.o:  $(obj)/piggy.gz FORCE


$(obj)/vmlinux.lds: $(obj)/vmlinux.lds.in arch/csky/boot/Makefile .config
	@sed "$(SEDFLAGS)" < $< > $@

