

SDK_VERSION			?=	3_4_0
CELL_MK_DIR 		?= $(CELL_SDK)/samples/mk

# -----------------------------------------------------------------

ifneq ($(LINUX),1)
ifneq ($(WIN32),1)
export CELL=1
export COLLADA_DOM_PLATFORM=CELL
endif
endif

PPU_CWARNFLAGS		?= $(CWARNFLAGS_L1)
PPU_CXXWARNFLAGS	?= $(CXXWARNFLAGS_L1)

include $(CELL_MK_DIR)/sdk.makedef.mk

## PSGL specific
PPU_LDLIBDIR		+= -L$(PPU_PSGL_LIBDIR) 

## FW specific
CELL_FW_DIR			?=$(CELL_SDK)/samples/fw

PPU_LDLIBDIR		+= -L$(CELL_FW_DIR) -Lsource/zlib

ifeq ($(CELL_PSGL_VERSION),gcmhud)
GRAPHICS_SAMPLE_CORE_LIBS = -lPSGLFX -lPSGL -lPSGLU -lgcm_hud -lgcm_pm \
							-lsysmodule_stub -lusbd_stub -lio_stub -lm \
							-lstdc++ -lsysutil_stub -lfs_stub -lsheap_stub \
							-ldbgfont -lresc_stub -lperf
else
GRAPHICS_SAMPLE_CORE_LIBS = -lPSGLFX -lPSGL -lPSGLU -lm -lusbd_stub -lfs_stub \
							-lio_stub -lsysutil_stub -ldbgfont -lresc_stub \
							-lgcm_cmd -lgcm_sys_stub -lsysmodule_stub -lperf
endif

PPU_INCDIRS			+= -I$(CELL_FW_DIR)/include

# build everything without exceptions so you can link with fw
PPU_CXXSTDFLAGS 	+= -DDATA_IN_HOME_DIR -D__CELLOS_LV2__
GCC_PPU_CXXSTDFLAGS += -fno-exceptions
GCC_PPU_LDFLAGS     += -fno-exceptions

## PSGL samples specific
COLLADA_DOM_DIR		= $(CELL_SDK)/samples/COLLADA_DOM

# where to build the graphics data to
CELL_GRAPHICS_DATA_DIR	= $(CELL_DATA_DIR)/graphics

ifeq ($(NO_OPTIMIZATIONS),1)
PPU_OPTIMIZE_LV = 
endif 

PPU_CSTDFLAGS 	+= -std=gnu99

# --------------------------------------------------------------

PPU_INCDIRS		+= 	-I$(CELL_FW_DIR)/include -Iinclude -I../include -Isource 
					
PPU_LIBS		+= $(CELL_FW_DIR)/libfw.a
PPU_LDLIBS		:= $(GRAPHICS_SAMPLE_CORE_LIBS) 

PPU_CPPFLAGS	+= -DPSGL
PPU_CXXSTDFLAGS	+= -fno-exceptions

PPU_SRCS		= 	source/main.cpp
ELF				=	elf
MY_TARGET		=	psgl_sample
PPU_TARGET		:= $(MY_TARGET).$(ELF)

include $(CELL_MK_DIR)/sdk.target.mk

run:
	ps3load $(MY_TARGET).self


