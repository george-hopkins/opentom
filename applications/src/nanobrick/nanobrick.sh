#! /bin/sh

echo ${0%/*}
cd ${0%/*}
exec ./nanobrick -s `pwd`/nanobrick_images.gif -l Premier\ Jeu
