pkgname="LANfiles"
pkgver="0.0.1-alpha1"
pkgrel="1"
pkgdesc="Simple program to send files over TCP"
arch=('i686' 'pentium4' 'x86_64' 'arm' 'armv7h' 'armv6h' 'aarch64')
license=('GPL-3.0-or-later')
url="https://github.com/ATcat-pl/lanfiles"
build() {
	cd "${srcdir}"
	ls
	gcc lanfiles.c -o lanfiles
}

package() {
	install -Dm 755 lanfiles "${pkgdir}/usr/bin/lanfiles"
}
