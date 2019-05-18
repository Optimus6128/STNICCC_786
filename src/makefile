obj = demo.obj video.obj timer.obj gfxtools.obj fx_anim.obj scene1.obj
bin = demo.exe

CPP = wpp386
CXXFLAGS = -fpc -otexan -oh -oi -ei -bt=DOS
LD = wlink

$(bin): $(obj)
    $(LD) name $@ file { *.obj } $(LDFLAGS)

.cpp.obj: .autodepend
    $(CPP) $(CXXFLAGS) $<

clean: .symbolic
    del *.obj
    del *.err
    del $(bin)
