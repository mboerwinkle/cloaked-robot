#!/bin/bash

rm sectors/*_*
echo 'entity
3
2
1
10000
10000
end' > sectors/0_0
echo "Server launched. Join to take out a carrier!!1"
./run
