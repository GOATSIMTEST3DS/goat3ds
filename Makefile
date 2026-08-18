.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

TARGET      :=  goat3ds
BUILD       :=  build
SOURCES     :=  source
DATA        :=  data
INCLUDES    :=  include

APP_TITLE       := Goat Simulator 3DS
APP_DESCRIPTION := Headbutt everything before the timer runs out
APP_AUTHOR      := Homebrew Port

export ICON         :=  $(CURDIR)/meta/icon.png
export BANNER_IMG   :=  $(CURDIR)/meta/banner.png
export BANNER_AUDIO :=  $(CURDIR)/meta/banner.wav
export RSF          :=  $(CURDIR)/meta/app.rsf

ARCH    :=  -march=armv6k -mtune=mpcore
