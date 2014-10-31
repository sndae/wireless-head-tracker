# set FLASH_SIZE_KB to either 16 or 32

FLASH_SIZE_KB = 32

ifeq ($(FLASH_SIZE_KB),32)
    CODE_SIZE = 0x8000
else ifeq ($(FLASH_SIZE_KB),16)
    CODE_SIZE = 0x4000
else
    $(error Please set FLASH_SIZE_KB to either 16 or 32)
endif

TARGET    = whtdngl
CFLAGS    = --model-large -I../tracker -I../mcu-lib -DNRF24LU1
CFLAGS   += -DDBG_MODE=0 -DPRINTF=printf_tiny
CFLAGS   += -DMATH_CORDIC
LFLAGS    = --code-loc 0x0000 --code-size $(CODE_SIZE) --xram-loc 0x8000 --xram-size 0x800
ASFLAGS   = -plosgff
RELFILES  = main.rel proc_packet.rel usb_desc.rel math_cordic.rel rf_dngl.rel usb.rel reports.rel dongle_settings.rel
RELFILES += rf_addr.rel nrfutils.rel nrfdbg.rel nRF24L.rel crtxinit.rel

VPATH    = ../mcu-lib:../tracker

$(TARGET).hex: $(RELFILES)
	sdcc $(CFLAGS) $(LFLAGS) $(RELFILES) -o $(TARGET).hex
	grep "ROM/EPROM" $(TARGET).mem

%.rel: %.c
	sdcc $(CFLAGS) -c $<

%.rel: %.asm
	sdas8051 $(ASFLAGS) $@ $< 

clean:
	rm -f *.hex *.lnk *.lst *.LST *.map *.rel *.rst *.sym *.mem *.lk *.asm *.lk *.cdb *.omf

all: clean $(TARGET).hex

flash: clean $(TARGET).hex
	nrfburn -f $(FLASH_SIZE_KB) -w $(TARGET).hex
