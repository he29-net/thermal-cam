# Simple Handheld Thermal Imager

This project contains HW design files and firmware for a cheap handheld 32x24 thermal imager based on the MLX90640 sensor, forked from https://gitee.com/qlexcel/thermal-cam The most common two-button MLX90640 product available at eBay or AliExpress at the end of 2022 seems to be using a similar board design and the same FW binary from the linked repository.

Board schematics were made in Altium designer and can be viewed in the free web viewer (https://www.altium.com/viewer/). I do not have access to Altium Designer and do not need another board, so I won't be making a version for KiCad or other free tool. But feel free to open a PR if you have lots of free time on your hands. :)

My goal is simply to make the firmware a bit more polished, flexible and up to date with recently available tools.

## Initial changes
 - sources cleaned up a bit and converted to UTF-8
 - disabled a few pieces of code to get program size under the linker limit of Keil v5 MDK non-commercial version
 - moved battery icon to the left (in preparation for other changes)

## Planned changes
 - runtime selectable FPS (default 4 FPS is often quite noisy, but 1 FPS is often too slow..)
 - rework drawing code
	- fix glitches at high temperatures
	- reduce code size to make room for other features
	- improve performance to allow 8 FPS display rate (16 Hz sampling)
 - support a newer compiler (or better, a FOSS toolchain, to get rid of the 32k limitation)
 - translate, add, and improve comments
 - different color mappings?

## How to build and use
 - get and install MDK-ARM 5.38a or similar version (https://www.keil.com/demo/eval/arm.htm)
 - get and install the legacy Arm Compiler 5 (https://developer.arm.com/downloads/view/ACOMP5)
	- if you get a weird file access error during instalation, run the .msi installer from data/ folder directly
	- make sure you install it to the MDK location (e.g. `Keil\_v5/ARM/ARM_Compiler_5.06u7/`), otherwise you will get licensing errors
 - open Keil uVision 5, install support package for GD32F103C8T6 and open the the ThermalCam project
 - in Project → Manage → Project items..., add the ARM Compiler 5 to Folders/Extensions as another compiler
 - build the project (F7)
 - connect ST-Link programmer to the board
 - use ST-Link Utility to back up the original firmware (in case your seller made changes you wish to keep)
 - back in Keil uVision, you can now load the program to your board (F8)

**DISCLAIMER**: These instructions may be outdated and may not work, may brick your device, and / or may open a gate to Hell or whatever if you don't know what you are doing. You know the drill.

## Original description in Chinese

----

![](/5.Docs/Images/ThermalCam.png)

# ThermalCam--红外热成像仪

**视频介绍：** https://www.bilibili.com/video/BV1634y1d7Ly/

## 0. 关于本项目
基于MLX90640传感器制作的红外热成像仪。

成本两百多，易于制作。

再也不用手感应测温了，被芯片烫伤、被高压电过的硬件狗飘过~~


## 1. 物料说明
**MLX90640**：分辨率为24x32的红外探头，如果只是测温，不想看像素级的温度已经足够。毕竟还有人用8x8分辨率的AMG8833。注意MLX90640 有两个型号， A 型和 B 型，型号全称为：MLX90640ESF_BAA/BAB。

![](/5.Docs/Images/MLX90640.png)

一定要选择B型。具体区别可以去看[我的博客](https://blog.csdn.net/qlexcel/article/details/119417088)。

**屏幕**：3.2寸，240x320分辨率。因为不同公司生成的屏幕引脚并不一定兼容，大伙们购买的时候要对清楚。我这里使用的是深圳艾斯迪科技的LCDT3213440AL。[购买链接](https://item.taobao.com/item.htm?spm=a1z09.2.0.0.6e602e8dW2vx0U&id=620056701505&_u=jdfumtd5f24)。买不带触摸屏的，我直接买的时候30块，现在......今年的电子行情懂的都懂......

**MCU**：使用GD32F103C8T6，不可以使用STM32F103C8T6。因为STM主频72M，GD主频108M，差了50%，就是这50%会导致视频显示特别卡，程序里面能优化的地方都优化了，C8T6的RAM只有20K，实际使用远远不够，因此程序里面很多变量做了共用存储空间处理，如果哪位大佬有兴趣可以继续优化下，或者直接上GD32F103CBT6。

**SPI FLASH**：使用的国产XT25F128BSSIGU，可以替换为华邦的或GD的，程序应该是兼容的。当然容量大小也可以更改。

**外壳**：外壳推荐在TB上买现成的来加工，规格为90x70x28，价格2元。[购买链接](https://item.taobao.com/item.htm?spm=a1z09.2.0.0.609c2e8dKVBPlZ&id=608913194349&_u=jdfumtdc14c)。加工的话需要美工刀和手钻。
![](/5.Docs/Images/Shell.png)


**锂电池**：我使用的是65mmx30mmx7mm，容量1800毫安的电池。电池可以使用更大的，因为外壳内部空间还有很大。

