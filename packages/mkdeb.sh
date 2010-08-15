#!/bin/sh
#
#Author: liuguangzhao@users.sf.net

appname=`basename $0 | sed s,\.sh$,,`
dirname=`dirname $0`
firstchar=`dirname $0|cut -c 1`
if [ "${firstchar}" != "/" ]; then
   dirname=$PWD/$dirname
fi

# pkgdir=karia2
VER_FILE=$dirname/../karia2.pro
echo $VER_FILE
# VERSION=`cat $VER_FILE|grep karia2|awk '{print $4}'`
VERSION=`cat $VER_FILE|grep "VERSION ="|head -n 1|awk '{print $3}'`
echo "Version: $VERSION"

SVNVER=0
ARCH=`uname -m`
if [ x"$1" != x"" ] ; then
	SVNVER=$1
fi
PKGDIR=/tmp/karia2-$VERSION-$SVNVER
rm -fvr /tmp/karia2-$VERSION-$SVNVER
mkdir -pv $PKGDIR

cp -va $dirname/debian/DEBIAN $PKGDIR/
rm -fvr $PKGDIR/DEBIAN/.svn

mkdir -pv $PKGDIR/usr/share/applications $PKGDIR/usr/bin $PKGDIR/usr/lib $PKGDIR/usr/share/karia2
cp -va $dirname/../data/karia2.desktop $PKGDIR/usr/share/applications/
cp -va $dirname/../karia2/bin $PKGDIR/usr/
cp -va $dirname/../karia2/lib $PKGDIR/usr/
rm -vf $PKGDIR/usr/lib/*.so.*

cp -va $dirname/../karia2/share/* $PKGDIR/usr/share/karia2/
find $PKGDIR/usr/share/karia2/ -name .svn | xargs rm -vfr

cd $PKGDIR/../
dpkg -b karia2-$VERSION-$SVNVER

mv -v /tmp/karia2-$VERSION-$SVNVER.deb /tmp/karia2-$VERSION-${SVNVER}_${ARCH}.deb

# cleanup
rm -fvr $PKGDIR

# come back
cd -
