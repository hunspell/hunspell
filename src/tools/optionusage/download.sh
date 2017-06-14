#!/usr/bin/env bash

PACKAGES=`apt-cache search hunspell|grep ^hunspell|grep dict|awk '{print $1}'|tr '\n' ' '`
echo $PACKAGES|sed 's/ /\n/g'

if [ -e usr ]
then
    rm -rf usr
fi
if [ -e var ]
then
    rm -rf var
fi

if [ -e debs ]
then
    rm -rf debs
fi
mkdir debs

cd debs
apt-get download $PACKAGES
for i in *.deb
do
    dpkg -x $i ..
done
cd ..

if [ -e debs ]
then
    rm -rf debs
fi
if [ -e var ]
then
    rm -rf var
fi
if [ -e usr/share/myspell ]
then
    rm -rf usr/share/myspell
fi
if [ -e usr/share/doc ]
then
    rm -rf usr/share/doc
fi
