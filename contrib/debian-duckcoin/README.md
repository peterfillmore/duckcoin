
Debian
====================
This directory contains files used to package duckcoind/duckcoin-qt
for Debian-based Linux systems. If you compile duckcoind/duckcoin-qt yourself, there are some useful files here.

## duckcoin: URI support ##


duckcoin-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install duckcoin-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your duckcoin-qt binary to `/usr/bin`
and the `../../share/pixmaps/bitcoin128.png` to `/usr/share/pixmaps`

duckcoin-qt.protocol (KDE)

