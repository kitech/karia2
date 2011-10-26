#!/bin/sh


appname=`basename $0 | sed s,\.sh$,,`
dirname=`dirname $0`
firstchar=`dirname $0|cut -c 1`
if [ "${firstchar}" != "/" ]; then
   dirname=$PWD/$dirname
fi

pkgdir=karia2

#LD_LIBRARY_PATH=$dirname:$dirname/lib
#export LD_LIBRARY_PATH
#$dirname/lib/$appname $*

mkdir -p $dirname/../$pkgdir
mkdir -p $dirname/../$pkgdir/bin
mkdir -p $dirname/../$pkgdir/lib
mkdir -p $dirname/../$pkgdir/share
mkdir -p $dirname/../$pkgdir/doc
mkdir -p $dirname/../$pkgdir/man
mkdir -p $dirname/../$pkgdir/plugins

rm -v $dirname/../$pkgdir/lib/*
cp -v $dirname/karia2 $dirname/../$pkgdir/bin/
chmod +x $dirname/../$pkgdir/bin/karia2

cp -v $dirname/../bin/karia2 $dirname/../$pkgdir/lib/

strip -s -v $dirname/../$pkgdir/lib/karia2

cp -Rv $dirname/../icons $dirname/../$pkgdir/share/
rm -vfr $dirname/../$pkgdir/share/icons/.svn
rm -vfr $dirname/../$pkgdir/share/icons/status/.svn
cp -Rv $dirname/../browser $dirname/../$pkgdir/share/
rm -vfr $dirname/../$pkgdir/share/browser/.svn
cp -Rv $dirname/../Resources $dirname/../$pkgdir/share/
rm -vfr $dirname/../$pkgdir/share/Resources/.svn
cp -Rv $dirname/../images $dirname/../$pkgdir/share/
rm -vfr $dirname/../$pkgdir/share/images/.svn
cp -Rv $dirname/../translations $dirname/../$pkgdir/share/
rm -vfr $dirname/../$pkgdir/share/translations/.svn

ln -sv ../share/icons $dirname/../$pkgdir/lib/
ln -sv ../share/browser $dirname/../$pkgdir/lib/
ln -sv ../share/Resources $dirname/../$pkgdir/lib/
ln -sv ../share/images $dirname/../$pkgdir/lib/
ln -sv ../share/translations $dirname/../$pkgdir/lib/

VER_FILE=$dirname/../karia2.pro
echo $VER_FILE
# VERSION=`cat $VER_FILE|grep karia2|awk '{print $4}'`
VERSION=`cat $VER_FILE|grep "VERSION ="|head -n 1|awk '{print $3}'`
echo "Version: $VERSION"

USED_OPENSSL_SSL=`ldd $dirname/../$pkgdir/lib/karia2|grep libssl|awk '{print $3}'` 
USED_OPENSSL_CRYPTO=`ldd $dirname/../$pkgdir/lib/karia2|grep libcrypto|awk '{print $3}'` 
USED_GSSAPI=`ldd $dirname/../$pkgdir/lib/karia2|grep libgssapi|awk '{print $3}'` 
USED_KRB5SO=`ldd $dirname/../$pkgdir/lib/karia2|grep libkrb5.so|awk '{print $3}'` 
USED_K5CRYPTO=`ldd $dirname/../$pkgdir/lib/karia2|grep libk5crypto|awk '{print $3}'` 
USED_KRB5SUPPORT=`ldd $dirname/../$pkgdir/lib/karia2|grep libkrb5support|awk '{print $3}'` 
USED_EXPAT=`ldd $dirname/../$pkgdir/lib/karia2|grep libexpat|awk '{print $3}'` 
USED_KEYUTILS=`ldd $dirname/../$pkgdir/lib/karia2|grep libkeyutils|awk '{print $3}'`
USED_SELINUX=`ldd $dirname/../$pkgdir/lib/karia2|grep libselinux|awk '{print $3}'`

echo $USED_OPENSSL_SSL $USED_OPENSSL_CRYPTO
cp -v $USED_OPENSSL_SSL $dirname/../$pkgdir/lib/
cp -v $USED_OPENSSL_CRYPTO $dirname/../$pkgdir/lib/

if [ x"$USED_GSSAPI" != x"" ] ; then
    cp -v $USED_GSSAPI $dirname/../$pkgdir/lib/
    cp -v $USED_KRB5SO $dirname/../$pkgdir/lib/
    cp -v $USED_K5CRYPTO $dirname/../$pkgdir/lib/
    cp -v $USED_KRB5SUPPORT $dirname/../$pkgdir/lib/
fi

if [ x"$USED_EXPAT" != x"" ] ; then
    cp -v $USED_EXPAT $dirname/../$pkgdir/lib/
fi
if [ x"$USED_KEYUTILS" != x"" ] ; then
    cp -v $USED_KEYUTILS $dirname/../$pkgdir/lib/
fi
if [ x"$USED_SELINUX" != x"" ] ; then
    cp -v $USED_SELINUX $dirname/../$pkgdir/lib/
fi


USED_QT=`ldd $dirname/../$pkgdir/lib/karia2|grep libQt|awk '{print $3}'`
echo $USED_QT
if [ x"$USED_QT" = x"" ]; then
    LINK_TYPE=static
else
    LINK_TYPE=shared
    for lib_name in $USED_QT 
    do
	#cp -v $lib_name $dirname/../$pkgdir/lib/
	echo $lib_name
    done
fi

#echo $USE_QTCORE $LINK_TYPE
#LINK_TYPE=static
LINK_PLATFORM=`uname`

TAR_CMD=tar
if [ x"${LINK_PLATFORM}" = x"SunOS" ]; then
    TAR_CMD=gtar
fi
if [ x"${LINK_PLATFORM}" = x"FreeBSD" ] ; then
	FBSDVER=`uname -r`
	FBSDVER=`echo $FBSDVER|cut -c 1`
	LINK_PLATFORM="$LINK_PLATFORM""$FBSDVER"    
	echo $LINK_PLATFORM
fi

MARCH=`uname -m`
echo "package info: $LINK_PLATFORM $LINK_TYPE $VERSION"
$TAR_CMD  jcvf $dirname/../karia2-$VERSION-$LINK_TYPE-qt4.$MARCH.$LINK_PLATFORM.tar.bz2 $dirname/../$pkgdir
