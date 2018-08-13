
Debian
====================
This directory contains files used to package sphinxcoind/sphinxcoin-qt
for Debian-based Linux systems. If you compile sphinxcoind/sphinxcoin-qt yourself, there are some useful files here.

## sphinxcoin: URI support ##


sphinxcoin-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install sphinxcoin-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your sphinxcoin-qt binary to `/usr/bin`
and the `../../share/pixmaps/sphinxcoin128.png` to `/usr/share/pixmaps`

sphinxcoin-qt.protocol (KDE)

