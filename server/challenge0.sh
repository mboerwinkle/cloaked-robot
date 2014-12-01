#!/bin/bash
rm sectors/*_*
echo 'entity
3
2
1
70000
64
end
entity
3
2
2
10000
64
end' > sectors/0_0
echo "Server launched. Join to help out the blue carrier!";
./run
