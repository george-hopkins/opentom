
./mkttimage ttsystem.cpio.z vmlinuz > ttsystem

ET

./ttimgextract ttsystem
gunzip < ttsystem.0 > ttsystem.cpio

Affichage de l'archive:

cpio -t -v < ./ttsystem.cpio

