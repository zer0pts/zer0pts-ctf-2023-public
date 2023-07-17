#!/bin/bash
export FLAG="zer0pts{Gr0up_r1ng_meow!!!}"
rm -rf distfiles
mkdir distfiles
sage ./challenge/q.sage > ./distfiles/output.txt
cp ./challenge/q.sage ./distfiles/
