# Maintainer: Jorge Martinez Hernandez <jorgemartinezhernandez@gmail.com>
# Contributor: Jorge Martinez Hernandez <jorgemartinezhernandez@gmail.com>

pkgname=feednix-git
pkgver=v0.6.4.1.1.rb716773
pkgrel=1
pkgdesc="A simple ncurses-based console client for Feedly"


arch=('any')
url="http://feednix-jarkore.rhcloud.com/"
license=('GPL3')
depends=('ncurses' 'curl' 'jsoncpp' 'w3m')
makedepends=('git')
md5sums=('SKIP')

source=('Feednix::git+http://github.com/Jarkore/Feednix')

pkgver() {
        cd "$srcdir/Feednix"
        git describe --long | sed -r 's/([^-]*g)/r\l/;s/-/./g'
}

build(){
        cd Feednix
        make
}

package(){
        cd "${srcdir}/Feednix"
        make install
        install -D -m755 config.json "${pkgdir}/etc/feednix/config.json"
        install -D -m755 feednix "${pkgdir}/usr/bin/feednix"
        install -D -m644 LICENSE "${pkgdir}/usr/share/licenses/feednix/LICENSE"
}
