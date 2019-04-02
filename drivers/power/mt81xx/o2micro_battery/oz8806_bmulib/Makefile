#ifeq ($(CONFIG_ARM64),y)
#obj-y += bmulib_64.o
#else
#obj-y += bmulib.o
#endif

obj-y += bmulib.o
#ldflags-y	+=  $(PWD)/kernel-3.18/drivers/misc/mediatek/power/mt6755/o2micro_battery/oz8806_bmulib/bmulib.o

### build bmulib to bmulib.ko
#obj-m += bmulibko.o
## replace bmulib.c with bmulib.o_shipped to build ko
#bmulibko-objs := bmulib.o
