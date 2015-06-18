#!/bin/bash
rm sectors/*_*
echo 'entity
3
2
1
4480000
1024
end
entity
3
2
2
160000
1024
end' > sectors/0_0
echo "Server launched. Join to help out the blue carrier!";
./run
