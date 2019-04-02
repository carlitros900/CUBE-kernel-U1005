ifeq ($(CONFIG_MEDIATEK_SOLUTION),y)
#    //include $(srctree)/drivers/misc/mediatek/Makefile.custom
endif
obj-y += bluewhale_charger.o

ifeq ($(CONFIG_OZ115_DUAL_SUPPORT),y)
	obj-y += oz115_sub.o
endif

obj-$(CONFIG_MEDIATEK_SOLUTION) += oz115_charger.o
