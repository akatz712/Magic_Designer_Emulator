To build Linux:
I run the script using GNU autotools ./mdprepare.sh which builds a dstribution file mdesigner-v1.v2.tar.gz with v1 and v2 being version and subversion.
```
aclocal
autoconf
automake --add-missing
./configure --prefix=/usr
make distcheck
```
I upload the distribution file to Sourceforge.

You download from https://sourceforge.net/projects/complexshapes/files/mdesigner-v1.v2.tar.gz/download ,and then extract it, and go to the folder in a terminal, and run the script ./mdinstall.sh as superuser.

Run ./mdremove.sh as superuser to remove the program.

For Linux, there are two dependencies, FLTK and libharu. These must be installed prior to building and using the program. The names for these packages in these Liunx distributions:

- **Fedora 29**			yum: gcc-c++, fltk, fltk-devel, libharu-devel
- **Lubuntu 18.10**		apt: libfltk1.3-dev, libhpdf-dev
- **Void base 32 bit**			base-devel, fltk-devel,	Haru-devel
- **Antergos base (Arch)**		fltk, libharu
- **Linux Mint 19.1 Mate**		build-essential, libfltk1.3-dev, libhpdf-dev
- **OpenSUSE Leap 15** 	gcc-c++, fltk-devel, libharu-devel
- **Debian stretch 9.3.0** 	build-essential, libfltk1.3-dev, libhpdf-dev

For MS Windows, I static linked everything to produce one exe file for portable distribution. The following steps:

0. Since my system is antergos (Arch) Linux, I found in the AUR -  https://aur.archlinux.org/packages/mingw-w64-gcc
Was tricky since I had to figure out what order to do the dependencies.
Note that it is possible to set up MinGW and Msys on a MS Windows system and do something similar to below.

1. The following external packages were download and their websites :

- https://www.fltk.org/software.php	fltk-1.3.5-source.tar.gz
- http://libharu.org/	libharu-libharu-RELEASE_2_3_0RC3-19-gd84867e.tar.gz

2. Each was extracted to its folder under Home, lets call them fltk and hpdf

3. For fltk: `./configure --host=x86_64-w64-mingw32`
4. `make`
5. If make stops before completing the tests, it is okay.
6. create include and lib folders under fltk/zlib
7. Copy/rename fltk/lib/libfltk_z.a → fltk/zlib/lib/libz.a
8. Copy fltk/zlib/*.h → fltk/zlib/include

9. For hpdf: `./buildconf.sh`
`./configure --host=x86_64-w64-mingw32 --with-zlib="/.../fltk/zlib" --without-png`
10. `make`

11. Finally, to build the executable DesignerFLagg.exe for MS Windows, do the following:
12. ```x86_64-w64-mingw32-g++ -c -Dwin32=1 `/.../fltk/fltk-config --cxxflags\` -I/.../hpdf/include *.cxx *.c agg2d/*.cpp```
13. ```x86_64-w64-mingw32-g++ *.o -static -static-libgcc -static-libstdc++ `/.../fltk/fltk-config --use-images --ldflags` /.../hpdf/src/.libs/libhpdf.a -s -no-pie -o DesignerFLagg.exe```
14. `rm *.o`

Note: remember to replace ... with the required path





