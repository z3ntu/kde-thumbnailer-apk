# KDE Thumbnailer APK

Based on https://store.kde.org/p/1081013/, updated and working for Qt 5.12.0 & KDE Frameworks 5.54.0.

## Installation

This lines will install it correctly for Arch Linux, be careful with other distros!
```
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX='/usr' -DCMAKE_INSTALL_LIBDIR=lib ..
make
sudo make install
```
