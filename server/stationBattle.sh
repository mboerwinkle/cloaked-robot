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
x=-128000
while [ $x -le 128000 ];
do y=-128000;
while [ $y -le 128000 ];
do makeAsteroid $(( $x + $1 )) $(( $y + $2 ));
let y+=32000;
done;
let x+=32000;
done;
}

rm sectors/*_*
echo 'entity
10
10
2
128000
0
end
entity
10
10
1
2528000
0
end
entity
9
9
2
128000
-128000
end
entity
9
9
1
2528000
-128000
end' > sectors/0_0
makeAsteroidField 128000 -320000
makeAsteroidField 2528000 -320000
echo "Server launched. Let's have us a space station battle.";
./run
