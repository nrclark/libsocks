CHROOT_DIR := chroot_dir

BIND_FLAGS := \
    --chown-deny --chgrp-deny --chmod-deny --xattr-ro \
    --delete-deny --rename-deny --realistic-permissions \
    --enable-ioctl \
    --no-allow-other \
    -o dev

RO_BINDS := bin etc proc sys usr lib lib64
RW_BINDS := tmp dev
ALL_BINDS := $(RO_BINDS) $(RW_BINDS)

BIND_FLAGS := $(strip $(BIND_FLAGS))

define \n


endef

prep-chroot:
	mkdir -p $(CHROOT_DIR)
	mkdir -p $(CHROOT_DIR)/workdir
	$(foreach x,$(ALL_BINDS),mkdir -p $(CHROOT_DIR)/$x$(\n))
	$(foreach x,$(RO_BINDS),bindfs $(BIND_FLAGS) -o ro /$x $(CHROOT_DIR)/$x$(\n))
	$(foreach x,$(RW_BINDS),bindfs $(BIND_FLAGS) /$x $(CHROOT_DIR)/$x$(\n))
	bindfs $(BIND_FLAGS) $(abspath .) $(CHROOT_DIR)/workdir

cleanup:
	$(foreach x,$(ALL_BINDS),fusermount -u $(CHROOT_DIR)/$x || true$(\n))
	$(foreach x,$(ALL_BINDS),fusermount -u $(CHROOT_DIR)/workdir || true$(\n))
	$(foreach x,$(ALL_BINDS),if [ -d $(CHROOT_DIR)/$x ]; then rmdir $(CHROOT_DIR)/$x; fi$(\n))
	find $(CHROOT_DIR) -maxdepth 1 -type f | xargs -r rm
	rm -rf $(CHROOT_DIR)/root
	if [ -d $(CHROOT_DIR)/workdir ]; then rmdir $(CHROOT_DIR)/workdir; fi

CHROOT_CMD ?= /bin/sh

run-chroot:
	fakechroot fakeroot $$(which chroot) $(CHROOT_DIR) $(CHROOT_CMD)
