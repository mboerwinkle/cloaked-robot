#!/bin/bash

rm sectors/*_*
echo 'entity
3
2
2
0
30000
end
entity
3
2
2
-8660
20000
end
entity
3
2
2
8660
20000
end
entity
7
7
2
-17320
10000
end
entity
7
7
2
0
10000
end
entity
7
7
2
17320
10000
end
entity
3
2
1
0
230000
end
entity
3
2
1
-8660
240000
end
entity
3
2
1
8660
240000
end
entity
7
7
1
-17320
250000
end
entity
7
7
1
0
250000
end
entity
7
7
1
17320
250000
end' > sectors/0_0
echo "Server launched. Easier battle than challenge 2, but bigger in scale."
./run
