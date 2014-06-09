# Maintainer: Jorge Martinez Hernandez <jorgemartinezhernandez@gmail.com>
# Contributor: Jorge Martinez Hernandez <jorgemartinezhernandez@gmail.com>

pkgname=feednix
pkgver=0.6
pkgrel=1
pkgdesc="A simple ncurses-based console client for Feedly"


arch=('any')
url="http://feednix-jarkore.rhcloud.com/"
license=('GPL3')
depends=('ncurses' 'curl' 'jsoncpp')
makedepends=('git' 'gcc')
md5sums=('SKIP')

source=('Feednix::git+http://github.com/Jarkore/Feednix')

pkgver() {
          cd "$srcdir/$pkgname"
            git describe --long | sed -r 's/([^-]*g)/r\l/;s/-/./g'

}

build(){
        cd Feednix
        make
}

package(){
        cd "${srcdir}/Feednix"
        make install
        make clean
}
