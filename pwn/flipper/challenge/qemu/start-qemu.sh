#!/bin/sh
LENGTH=9
STRENGTH=27
challenge=`dd bs=32 count=1 if=/dev/urandom 2>/dev/null | base64 | tr +/ _. | cut -c -$LENGTH`
echo hashcash -mb$STRENGTH $challenge

echo "hashcash token: "
read token
if [ `expr "$token" : "^[a-zA-Z0-9\+\_\.\:\/]\{52\}$"` -eq 52 ]; then
    hashcash -cdb$STRENGTH -f /tmp/hashcash.sdb -r $challenge $token 2> /dev/null
    if [ $? -eq 0 ]; then
        echo "[+] Correct"
    else
        echo "[-] Wrong"
        exit
    fi
else
    echo "[-] Wrong"
    exit
fi

cd /home/user/flipper/challenge/qemu

timeout --foreground 300 qemu-system-x86_64 \
    -m 64M \
    -nographic \
    -kernel bzImage \
    -append "rootwait root=/dev/vda rw init=/init console=ttyS0 quiet oops=panic panic_on_warn=1 panic=-1" \
    -no-reboot \
    -cpu kvm64,+smap,+smep \
    -monitor /dev/null \
    -drive file=rootfs.ext2,if=virtio,format=raw \
    -snapshot
