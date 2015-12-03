#!/bin/bash
rm sectors/*_*
echo 'entity
3
3
1
160000
160000
end' > sectors/0_0
echo "Server launched. Join to take out a carrier!!1"
./run
