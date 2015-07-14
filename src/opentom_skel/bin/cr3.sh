#! /bin/sh 

# make CR Engine root link
test -d /root || { \
	mount -oremount,rw /; \
	ln -s $DIST/share/cr3/root /root; \
	mount -oremount,ro /; \
}

f=`flchoose -e '*.epub' -t 'Please choose a file to open with CoolReader (...and wait)'`
if [ ! -z "$f" ]; then
        exec cr3 "$f" >$DIST/logs/cr3.log 2>&1
fi

