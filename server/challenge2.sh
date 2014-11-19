#!/bin/bash

rm sectors/*_*
echo 'entity
7
7
1
100000
-4000
end
entity
3
2
1
100000
0
end
entity
3
2
2
10000
-1000
end' > sectors/0_0
echo "Server launched. Carrier v. carrier, but they have something... new...";
./run
