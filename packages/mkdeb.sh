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

PKGDIR=/tmp/karia2-$VERSION-0
rm -fvr /tmp/karia2-$VERSION-0
mkdir -pv $PKGDIR

cp -va debian/DEBIAN $PKGDIR/
rm -fv $PKGDIR/DEBIAN/.svn

mkdir -pv $PKGDIR/usr/share/applications $PKGDIR/usr/bin $PKGDIR/usr/lib $PKGDIR/usr/share/karia2
cp -va ../data/karia2.desktop $PKGDIR/usr/share/applications/
cp -va ../karia2/bin $PKGDIR/usr/
cp -va ../karia2/lib $PKGDIR/usr/

cp -va ../karia2/share/* $PKGDIR/usr/share/karia2/
find $PKGDIR/usr/share/karia2/ -name .svn | xargs rm -vfr

cd $PKGDIR/../
dpkg -b karia2-$VERSION-0

# cleanup
rm -fvr $PKGDIR

# come back
cd -
