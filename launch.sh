#!/bin/sh

export PKG_CONFIG_PATH=/home/red/Samba/debian_yy3568/YY3568-Debian10/buildroot/output/rockchip_rk3568/host/aarch64-buildroot-linux-gnu/sysroot/usr/lib/pkgconfig/:/home/red/Samba/debian_yy3568/YY3568-Debian10/buildroot/output/rockchip_rk3568/host/aarch64-buildroot-linux-gnu/sysroot/usr/share/pkgconfig/

export PATH=/home/red/Samba/debian_yy3568/YY3568-Debian10/buildroot/output/rockchip_rk3568/host/bin:$PATH
cmake -DCMAKE_TOOLCHAIN_FILE=../cross_compile.cmake ..
