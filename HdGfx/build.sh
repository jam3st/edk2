#!/bin/sh 

bd=${PWD}/build
cc=/usr/x86_64-pc-linux-gnu/gcc-bin/7.3.1/x86_64-pc-linux-gnu-gcc
ar=/usr/x86_64-pc-linux-gnu/gcc-bin/7.3.1/x86_64-pc-linux-gnu-gcc-ar
rm -rf ${bd}
mkdir -p build/adalib build/adainclude build/lib/gnat

##### GNAT #####
cp gnat/*.ad? ${bd}/adainclude
for i in gnat/ada.ads gnat/a-unccon.ads gnat/gnat.ads gnat/g-souinf.ads gnat/interfac.ads gnat/s-atacco.ads gnat/s-maccod.ads gnat/s-parame.ads gnat/s-unstyp.ads gnat/system.ads ; do
    ${cc} --RTS=${bd} -gnatg -gnatpg -m64 -fuse-ld=bfd -fno-stack-protector -Wl,--build-id=none -march=x86-64 -c ${i} -o ${bd}/lib/`dirname ${i}`/`basename -s .ads ${i}`.o
done


for i in gnat/i-c.adb gnat/s-imenne.adb gnat/s-stoele.adb ; do
    ${cc} --RTS=${bd} -gnatg -gnatpg -m64 -fuse-ld=bfd -fno-stack-protector -Wl,--build-id=none -march=x86-64 -c ${i} -o ${bd}/lib/`dirname ${i}`/`basename -s .adb ${i}`.o
done

cp ${bd}/lib/gnat/ada.ali ${bd}/lib/gnat/a-unccon.ali ${bd}/lib/gnat/gnat.ali ${bd}/lib/gnat/g-souinf.ali ${bd}/lib/gnat/i-c.ali ${bd}/lib/gnat/interfac.ali ${bd}/lib/gnat/s-atacco.ali ${bd}/lib/gnat/s-imenne.ali ${bd}/lib/gnat/s-maccod.ali ${bd}/lib/gnat/s-parame.ali ${bd}/lib/gnat/s-stoele.ali ${bd}/lib/gnat/s-unstyp.ali ${bd}/lib/gnat/system.ali ${bd}/adalib

${ar} cr ${bd}/libgnat.a ${bd}/lib/gnat/ada.o ${bd}/lib/gnat/a-unccon.o ${bd}/lib/gnat/gnat.o ${bd}/lib/gnat/g-souinf.o ${bd}/lib/gnat/i-c.o ${bd}/lib/gnat/interfac.o ${bd}/lib/gnat/s-atacco.o ${bd}/lib/gnat/s-imenne.o ${bd}/lib/gnat/s-maccod.o ${bd}/lib/gnat/s-parame.o ${bd}/lib/gnat/s-stoele.o ${bd}/lib/gnat/s-unstyp.o ${bd}/lib/gnat/system.o
##### GNAT #####
##### HWBASE #####
mkdir ${bd}/libhwbase
for i in glue/hw-debug_sink.adb libhwbase/ada/dynamic_mmio/hw-mmio_range.adb libhwbase/debug/hw-debug.adb \
    libhwbase/common/direct/hw-pci-dev.adb libhwbase/common/hw-mmio_regs.adb libhwbase/common/hw-pci-mmconf.adb \
    libhwbase/common/hw-port_io.adb libhwbase/common/hw-time.adb ; do
    mkdir -p ${bd}/`dirname ${i}`
    ${cc} --RTS=${bd} -gnatp -m64 -fuse-ld=bfd -fno-stack-protector -Wl,--build-id=none -ffunction-sections -fdata-sections -gnatwa.eeD.HHTU.U.W.Y -gnatyN -Os -gnata -march=x86-64 \
    -Iglue -Ilibhwbase/common -Ilibhwbase/debug -Ilibhwbase/ada/dynamic_mmio \
    -c ${i} -o ${bd}/`dirname ${i}`/`basename -s .adb ${i}`.o
done
for i in libhwbase/common/hw-pci.ads libhwbase/common/hw-sub_regs.ads libhwbase/common/hw.ads libhwbase/common/hw-config.ads ; do
    mkdir -p ${bd}/`dirname ${i}`
    ${cc} --RTS=${bd} -gnatp -m64 -fuse-ld=bfd -fno-stack-protector -Wl,--build-id=none -ffunction-sections -fdata-sections -gnatwa.eeD.HHTU.U.W.Y -gnatyN -Os -gnata -march=x86-64 \
    -Iglue -Ilibhwbase/common -Ilibhwbase/debug -Ilibhwbase/ada/dynamic_mmio \
    -c ${i} -o ${bd}/`dirname ${i}`/`basename -s .ads ${i}`.o
done
##### HWBASE #####
##### LIBGFXINIT #####
mkdir ${bd}/libgfxinit
for i in libgfxinit/common/haswell_shared/hw-gfx-gma-connectors-ddi.adb libgfxinit/common/haswell_shared/hw-gfx-gma-connectors-ddi.adb libgfxinit/common/haswell_shared/hw-gfx-gma-port_detect.adb \
    libgfxinit/common/haswell_shared/hw-gfx-gma-power_and_clocks_haswell.adb libgfxinit/common/haswell/hw-gfx-gma-connectors-ddi-buffers.adb \
    libgfxinit/common/haswell/hw-gfx-gma-plls-wrpll.adb libgfxinit/common/haswell/hw-gfx-gma-plls.adb libgfxinit/common/haswell/hw-gfx-gma-spll.adb \
    libgfxinit/common/hw-gfx-dp_aux_ch.adb libgfxinit/common/hw-gfx-dp_info.adb libgfxinit/common/hw-gfx-dp_training.adb libgfxinit/common/hw-gfx-edid.adb libgfxinit/common/hw-gfx-framebuffer_filler.adb \
    libgfxinit/common/hw-gfx-gma-config_helpers.adb libgfxinit/common/hw-gfx-gma-connector_info.adb libgfxinit/common/hw-gfx-gma-display_probing.adb libgfxinit/common/hw-gfx-gma-dp_aux_request.adb \
    libgfxinit/common/hw-gfx-gma-i2c.adb libgfxinit/common/hw-gfx-gma-panel.adb libgfxinit/common/hw-gfx-gma-pch-fdi.adb libgfxinit/common/hw-gfx-gma-pch-sideband.adb \
    libgfxinit/common/hw-gfx-gma-pch-transcoder.adb libgfxinit/common/hw-gfx-gma-pch-vga.adb libgfxinit/common/hw-gfx-gma-pipe_setup.adb libgfxinit/common/hw-gfx-gma-registers.adb \
    libgfxinit/common/hw-gfx-gma-transcoder.adb libgfxinit/common/hw-gfx-gma.adb ; do
    mkdir -p ${bd}/`dirname ${i}`
    ${cc} --RTS=${bd} -gnatp -m64 -fuse-ld=bfd -fno-stack-protector -Wl,--build-id=none -ffunction-sections -fdata-sections -gnatwa.eeD.HHTU.U.W.Y -gnatyN -Os -gnata -march=x86-64 \
    -Ilibhwbase/common/direct -Ilibhwbase/ada/dynamic_mmio -Ilibhwbase/common -Ilibhwbase/debug -Ilibgfxinit/common/haswell_shared -Ilibgfxinit/common/haswell -Ilibgfxinit/common \
    -c ${i} -o ${bd}/`dirname ${i}`/`basename -s .adb ${i}`.o
done

for i in libgfxinit/common/haswell/hw-gfx-gma-ddi_phy.ads libgfxinit/common/haswell_shared/hw-gfx-gma-ddi_phy_stub.ads libgfxinit/common/haswell/hw-gfx-gma-ddi_phy.ads \
    libgfxinit/common/haswell/hw-gfx-gma-plls-lcpll.ads libgfxinit/common/haswell/hw-gfx-gma-power_and_clocks.ads libgfxinit/common/hw-gfx-dp_defs.ads libgfxinit/common/hw-gfx-gma-dp_aux_ch.ads \
    libgfxinit/common/hw-gfx-gma-dp_info.ads libgfxinit/common/hw-gfx-i2c.ads libgfxinit/common/hw-gfx.ads libgfxinit/common/hw-gfx-gma-config.ads ; do
    mkdir -p ${bd}/`dirname ${i}`
    ${cc} --RTS=${bd} -gnatp -m64 -fuse-ld=bfd -fno-stack-protector -Wl,--build-id=none -ffunction-sections -fdata-sections -gnatwa.eeD.HHTU.U.W.Y -gnatyN -Os -gnata -march=x86-64 \
    -Ilibhwbase/common/direct -Ilibhwbase/ada/dynamic_mmio -Ilibhwbase/common -Ilibhwbase/debug -Ilibgfxinit/common/haswell_shared -Ilibgfxinit/common/haswell -Ilibgfxinit/common \
    -c ${i} -o ${bd}/`dirname ${i}`/`basename -s .ads ${i}`.o
done
