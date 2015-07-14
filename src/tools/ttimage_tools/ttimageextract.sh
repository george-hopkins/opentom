#!/bin/sh
CWD=$(pwd)
ORGTTIMAGE=${1:-"${CWD}/ttsystem"}
OUTPUT_DIR=${2:-"${CWD}/ttsystem_root"}
NAME=$(basename $0)

usage ()
{ [ "$*" != "" ] && echo $*
  cat << EOF

Usage:
$NAME [ttsystem image path] [extract directory]

If [ttsystem image path] is not specified ./ttsystem_root will be used.
If [extract directory] is not specified ./ttsystem_root will be used.

[extract directory] must not exist before running this script.
EOF
  exit 1
}

[ "$1" = "-h" ] && usage
[ ! -r $ORGTTIMAGE ] && usage "Error: No ttsystem found"
if [ -d $OUTPUT_DIR ]
then
  cat << EOF
Directory $OUTPUT_DIR already exists ...
Please remove it before running this or change destination directory.
EOF
  exit 1
else
  mkdir -p $OUTPUT_DIR
fi

IMAGE_SIZE=$(od -l -j4 -N4 $ORGTTIMAGE |head -1 |awk '{print $NF}')
cd $OUTPUT_DIR
dd if=$ORGTTIMAGE bs=1 skip=12 count=$IMAGE_SIZE |gunzip -c | cpio -id

