#---------------------------------------------------------------------------------
# Goat Simulator - 3DS homebrew Makefile
# Requires devkitARM + libctru + citro2d (installed via devkitPro pacman)
#---------------------------------------------------------------------------------
.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

#---------------------------------------------------------------------------------
# TARGET   : name of the .3dsx / .cia output
# BUILD    : intermediate build directory
# SOURCES  : source code directories
# DATA     : data directories
# INCLUDES : include directories
#---------------------------------------------------------------------------------
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

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH    :=  -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS  :=  -g -Wall -O2 -mword-relocations \
            -fomit-frame-pointer -ffunction-sections \
            $(ARCH)

CFLAGS  +=  $(INCLUDE) -DARM11 -D_3DS

CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

ASFLAGS :=  -g $(ARCH)
LDFLAGS =   -specs=3dsx_specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS    :=  -lcitro2d -lcitro3d -lctru -lm

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level
# containing include and lib
#---------------------------------------------------------------------------------
LIBDIRS := $(CTRULIB)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT   :=  $(CURDIR)/$(TARGET)
export TOPDIR   :=  $(CURDIR)

export VPATH    :=  $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                     $(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR  :=  $(CURDIR)/$(BUILD)

CFILES      :=  $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES    :=  $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES      :=  $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES    :=  $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
    export LD   :=  $(CC)
else
    export LD   :=  $(CXX)
endif

export OFILES_BIN  :=  $(addsuffix .o,$(BINFILES))
export OFILES_SRC   :=  $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES       :=  $(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN   :=  $(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE  :=  $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                     $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                     -I$(CURDIR)/$(BUILD)

export LIBPATHS :=  $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).3dsx $(TARGET).elf $(TARGET).smdh $(OUTPUT).3dsx \
	        $(TARGET).cia $(TARGET).icn $(TARGET).bnr

#---------------------------------------------------------------------------------
else
.PHONY: all

DEPENDS :=  $(OFILES:.o=.d)

#---------------------------------------------------------------------------------
all  :   $(OUTPUT).3dsx

$(OUTPUT).3dsx  :   $(OUTPUT).elf
$(OUTPUT).elf   :   $(OFILES)

#---------------------------------------------------------------------------------
# CIA build - requires bannertool and makerom (devkitPro 'general-tools' package)
#   make cia
#---------------------------------------------------------------------------------
.PHONY: cia

cia: $(OUTPUT).cia

$(OUTPUT).icn: $(ICON)
	bannertool makesmdh -i $(ICON) -s "Goat Simulator 3DS" \
	    -l "Headbutt everything before the timer runs out" \
	    -p "Homebrew Port" -o $@

$(OUTPUT).bnr: $(BANNER_IMG) $(BANNER_AUDIO)
	bannertool makebanner -i $(BANNER_IMG) -a $(BANNER_AUDIO) -o $@

$(OUTPUT).cia: $(OUTPUT).elf $(OUTPUT).icn $(OUTPUT).bnr
	makerom -f cia -o $@ -DAPP_ENCRYPTED=false -target t \
	    -exefslogo -elf $(OUTPUT).elf -rsf $(RSF) \
	    -icon $(OUTPUT).icn -banner $(OUTPUT).bnr \
	    -major 1 -minor 0 -micro 0

-include $(DEPENDS)

endif
#---------------------------------------------------------------------------------
