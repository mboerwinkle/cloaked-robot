#!/bin/bash

makeAsteroid () {
echo 'entity
4
3
0' >> sectors/0_0
echo $1 >> sectors/0_0
echo $2 >> sectors/0_0
echo end >> sectors/0_0
}

makeAsteroidField () {
x=-8000
while [ $x -le 8000 ];
do y=-8000;
while [ $y -le 8000 ];
do makeAsteroid $(( $x + $1 )) $(( $y + $2 ));
let y+=2000;
done;
let x+=2000;
done;
}

rm sectors/*_*
echo 'entity
10
10
2
8000
0
end
entity
10
10
1
158000
0
end
entity
9
9
2
8000
-8000
end
entity
9
9
1
158000
-8000
end' > sectors/0_0
makeAsteroidField 8000 -20000
makeAsteroidField 158000 -20000
echo "Server launched. Let's have us a space station battle.";
./run
