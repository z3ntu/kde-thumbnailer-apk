# KDE Thumbnailer APK

Based on https://store.kde.org/p/1081013/, updated and working for Qt 5.12.0 & KDE Frameworks 5.54.0.

It is currently only possible to parse APKs using the v1 signing schema (JAR signing). APKs signed with the APK Signature Scheme v2 or later fail because [KZip is unable to handle extra data before the Central Directory block in the file](https://bugs.kde.org/show_bug.cgi?id=415221).

## Installation

This lines will install it correctly for Arch Linux, be careful with other distros!
```
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX='/usr' -DCMAKE_INSTALL_LIBDIR=lib ..
make
sudo make install
```
