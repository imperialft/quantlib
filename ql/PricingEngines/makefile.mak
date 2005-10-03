
.autodepend
.silent

MAKE = $(MAKE)

!ifdef _DEBUG
!ifndef _RTLDLL
    _D = -sd
!else
    _D = -d
!endif
!else
!ifndef _RTLDLL
    _D = -s
!endif
!endif

!ifdef __MT__
    _mt = -mt
!endif

# Directories
INCLUDE_DIR    = ..\..

# Object files
OBJS = \
    "americanpayoffatexpiry.obj$(_mt)$(_D)" \
    "americanpayoffathit.obj$(_mt)$(_D)" \
    "blackformula.obj$(_mt)$(_D)" \
    "greeks.obj$(_mt)$(_D)" \
    "Asian\AsianEngines$(_mt)$(_D).lib" \
    "Barrier\BarrierEngines$(_mt)$(_D).lib" \
    "Basket\BasketEngines$(_mt)$(_D).lib" \
    "CapFloor\CapFloorEngines$(_mt)$(_D).lib" \
    "Cliquet\CliquetEngines$(_mt)$(_D).lib" \
    "Swaption\SwaptionEngines$(_mt)$(_D).lib" \
    "Vanilla\VanillaEngines$(_mt)$(_D).lib"


# Tools to be used
CC        = bcc32
TLIB      = tlib

# Options
CC_OPTS        = -vi- -q -c -I$(INCLUDE_DIR) -w-8070

!ifdef _DEBUG
    CC_OPTS = $(CC_OPTS) -v -D_DEBUG
!else
    CC_OPTS = $(CC_OPTS) -O2 -DNDEBUG
!endif

!ifdef _RTLDLL
    CC_OPTS = $(CC_OPTS) -D_RTLDLL
!endif

!ifdef __MT__
    CC_OPTS = $(CC_OPTS) -tWM
!endif

!ifdef SAFE
    CC_OPTS = $(CC_OPTS) -DQL_EXTRA_SAFETY_CHECKS
!endif

TLIB_OPTS    = /P128
!ifdef _DEBUG
TLIB_OPTS    = /P128
!endif

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj$(_mt)$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
PricingEngines$(_mt)$(_D).lib:: SubLibraries $(OBJS)
    if exist PricingEngines$(_mt)$(_D).lib    del PricingEngines$(_mt)$(_D).lib
    $(TLIB) $(TLIB_OPTS) "PricingEngines$(_mt)$(_D).lib" /a $(OBJS)

SubLibraries:
    cd Asian
    $(MAKE)
    cd ..\Barrier
    $(MAKE)
    cd ..\Basket
    $(MAKE)
    cd ..\CapFloor
    $(MAKE)
    cd ..\Cliquet
    $(MAKE)
    cd ..\Swaption
    $(MAKE)
    cd ..\Vanilla
    $(MAKE)
    cd ..


# Clean up
clean::
    if exist *.obj* del /q *.obj*
    if exist *.lib  del /q *.lib
    cd Asian
    $(MAKE) clean
    cd ..\Barrier
    $(MAKE) clean
    cd ..\Basket
    $(MAKE) clean
    cd ..\CapFloor
    $(MAKE) clean
    cd ..\Cliquet
    $(MAKE) clean
    cd ..\Swaption
    $(MAKE) clean
    cd ..\Vanilla
    $(MAKE) clean
    cd ..
