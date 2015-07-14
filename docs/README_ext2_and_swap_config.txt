How doing your OpenTom ext2/swap enabled SD (for Navit)

Make sure you use the good SDcard partition, you can erase all your Linux system if not !!!
(plug it in your computer and use 'mount' command to see which is on /media/<the name/id of my SD>)

1)========= Make your partition

You need 3 partitions, the first with maximum size, 2nd and thrd only ~32Mo.

With fdisk:
sudo fdisk /dev/<SDCARD device>		# without final number

Then type:
'c'			# to disable MSDOS compatibility
'd' <x>			# where <x> is each existing partitions to be deleted
'n' '1'	[ENTER] +31M	# to create a ~32M partition
'p'			# to print the result (like sample bellow)

>Commande (m pour l'aide): p
>
>Disque /dev/sdd: 4006 Mo, 4006608896 octets
>53 têtes, 52 secteurs/piste, 2839 cylindres
>Unités = cylindres de 2756 * 512 = 1411072 octets
>Sector size (logical/physical): 512 bytes / 512 bytes
>I/O size (minimum/optimal): 512 bytes / 512 bytes
>Identifiant de disque : 0x7fc556bf
>
>Périphérique Amorce  Début        Fin      Blocs     Id  Système
>/dev/sdd1               1          23       30669+  83  Linux


You know now that you need 23 block to make a ~32Mo partition size. Then :

'd' '1'			# to remove the test size partition
'n' 'p' '1' [ENTER] 	# to start the creation of the new partition
<number>		# where <number> is the max proposed minus 2xthe size of 32M (=46)

'n' 'p' '2' [ENTER] +23	# to create swap partition
't' '2' 'b'		# change the partition 2 to type swap

'n' 'p' '3' 2 x [ENTER] # to create linux ext2 boot partition

* The result does look like:

> Disk /dev/sdd: 4006 MB, 4006608896 bytes
> 53 heads, 52 sectors/track, 2839 cylinders
> Units = cylinders of 2756 * 512 = 1411072 bytes
> Sector size (logical/physical): 512 bytes / 512 bytes
> I/O size (minimum/optimal): 512 bytes / 512 bytes
> Disk identifier: 0x00000000

>   Device Boot      Start         End      Blocks   Id  System
> /dev/sdd1               1        2791     3844974    b  W95 FAT32
> /dev/sdd2            2792        2815       33072   82  Linux swap / Solaris
> /dev/sdd3            2816        2839       33072   83  Linux


2)========= Format the partitions

sudo mkfs.vfat -F32 -nOPENTOM /dev/<SDCARD device>1
sudo mkswap -Lswap /dev/<SDCARD device>2
sudo mkfs.ext2 -Lext2_boot /dev/<SDCARD device>3

sync
sync
(unplug/replug your SD card)


3)========= Install all you need in new partitions

in OpenTomSDK directory :

cp configs/busybox_config.pivot_root build/busybox-1.22.1/.config
cp configs/kernel_config.ext2 kernel/.config
make ttsystem

sudo cp -R initramfs/* /media/ext2_boot/
sudo cp configs/etc_rc_file.ext2 /media/ext2_boot/etc/rc
cd /media/ext2_boot/var && sudo rm -Rf log run tmp
sudo ln -s /tmp log
sudo ln -s /tmp run
sudo ln -s /tmp tmp
cd -

sudo cp configs/etc_rc_file.pivot_root_ext2 initramfs/etc/rc
make ttsystem
cp build/ttsystem /media/OPENTOM/


4)======== If it goes wrong

If it doesn't boot, use config/kernel_config.console_ext2 to debug the boot.

If ext2 part make i/o errors with your TomTom, link (in ext2 part /log/* to -> /tmp) and don't mount r/w the ext2 part in /media/ext2_boot/etc/rc file.

Some times at boot, the ethernet gadget kernel module block when the USB cable is connected to the computer, then just unplug/replug it to continue the boot process ...





