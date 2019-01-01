#! /bin/sh

export HOME="$DIST"

# Login files
[ -f "$DIST/etc/.shinit" ] && cp -f "$DIST/etc/.shinit" /etc/
[ -f "$DIST/etc/profile" ] && cp -f "$DIST/etc/profile" /etc/
. /etc/.shinit

DB=/etc/dropbear
RSA_KEYFILE="$DB/dropbear_rsa_host_key"

echo 'Disabling BT'
stop_bt -s

#echo 'syslogd'
#rm -f "$DIST/logs/messages"; syslogd "-O$DIST/logs/messages" -l8

echo 'opentom * opentom' >/etc/ppp/pap-secrets
chmod 0600 /etc/ppp/pap-secrets

ifconfig lo 127.0.0.1 up

echo 'telnetd'
telnetd

echo 'ssh private key'
[ -d "$DIST$DB" ] && {
	[ -e "$DIST$RSA_KEYFILE" ] || dropbearkey -t rsa -f "$DIST$RSA_KEYFILE" &&
	cp -f "$DIST$RSA_KEYFILE" "$RSA_KEYFILE" &&
	chmod 0600 "$RSA_KEYFILE"
}

echo 'ssh server'
[ -f "$RSA_KEYFILE" ] && [ -f "$DIST$DB/authorized_keys" ] &&
cp -f "$DIST$DB/authorized_keys" /.ssh/authorized_keys && chmod 0600 /.ssh/authorized_keys
env HOME=/ dropbear -sgB

# Verify TS is calibrated
[ -f "$TSLIB_CALIBFILE" ] || ts_calibrate

# Suspend when the power button is pressed or the battery is low
power_button -b "$DIST/bin/suspend" "$DIST/bin/suspend" &

while true
do
	pidof nano-X >/dev/null 2>&1 || { nice -n -10 nano-X & }
	pidof nanowm >/dev/null 2>&1 ||	{ nanowm & }
	pidof nano-X >/dev/null 2>&1 && pidof nanowm >/dev/null 2>&1 || ! sleep 1 || continue
	nxmenu "$DIST/etc/nxmenu.cfg" >"$DIST/logs/nxmenu.log" 2>&1
done
