# SPDX-License-Identifier: GPL-2.0

ifeq ($(CONFIG_MACH_XIAOMI),y)

# CAF convention: the AOSP build collects these into dtb-y / dtbo-y.
ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
	dtb-$(CONFIG_MACH_XIAOMI) += \
		sm8150-xiaomi.dtb \
		sm8150-v2-xiaomi.dtb

	dtbo-$(CONFIG_MACH_XIAOMI_VAYU) += \
		vayu-sm8150-overlay.dtbo

	dtbo-$(CONFIG_MACH_XIAOMI_RAPHAEL) += \
		raphael-sm8150-overlay.dtbo

vayu-sm8150-overlay.dtbo-base := sm8150-xiaomi.dtb sm8150-v2-xiaomi.dtb
raphael-sm8150-overlay.dtbo-base := sm8150-xiaomi.dtb sm8150-v2-xiaomi.dtb
endif

# Nothing above is reachable by a plain `make dtbs`: CONFIG_MACH_XIAOMI_* is
# never assigned by the Kconfig below, so the CAF blocks select nothing, and
# the tree had no dtb-y/dtbo-y at all. Declare the raphael targets directly so
# a standalone `make Image dtbs` produces what the device needs:
#   sm8150-xiaomi.dtb          -> ships inside vendor_boot.img (boot header v3
#                                 carries no DTB in boot.img; the bootloader
#                                 passes this one in x0)
#   raphael-sm8150-overlay.dtbo -> ships inside dtbo.img
ifeq ($(CONFIG_MACH_XIAOMI_RAPHAEL),y)
	dtb-y += sm8150-xiaomi.dtb
ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
	dtbo-y += raphael-sm8150-overlay.dtbo
endif
endif

endif

always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
