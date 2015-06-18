#!/bin/bash
rm sectors/*_*
echo 'entity
7
7
1
1600000
64000
end
entity
7
7
1
1600000
-64000
end
entity
3
2
1
1600000
0
end
entity
3
2
2
160000
-16000
end' > sectors/0_0
echo "Server launched. Carrier v. carrier, but they have something... new...";
./run
