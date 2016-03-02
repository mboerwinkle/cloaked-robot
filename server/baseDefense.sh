#!/bin/bash

#Bash is a bit weird. The syntax for making a function looks like there are no arguments, but actually every function has arbitrarily many args. Maybe we should move to, like, python or lisp or something for this scripting. IDK.
makeAsteroid () {
echo 'entity
12
4
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
let y+=64000;
done;
let x+=64000;
done;
}

makeColony () {
echo 'entity
10
10' >> sectors/0_0
echo $1 >> sectors/0_0
echo $2 >> sectors/0_0
echo $3 >> sectors/0_0
echo 'end
entity
9
9' >> sectors/0_0
echo $1 >> sectors/0_0
echo $2 >> sectors/0_0
echo $(( $3 - 128000 )) >> sectors/0_0
echo 'end
entity
9
9' >> sectors/0_0
echo $1 >> sectors/0_0
echo $2 >> sectors/0_0
echo $(( $3 + 128000 )) >> sectors/0_0
echo end >> sectors/0_0
}

# * 128 would give the full range of a sector, but that's not dense enough
pos () {
	echo $(( ($RANDOM - 16384) * 90 * 16 ))
}

rm sectors/*_*

makeColony 2 0 0

for (( i=0; i<3; i++ ))
do
	makeColony 1 `pos` `pos`
	makeColony 3 `pos` `pos`
done

for (( i=0; i<25; i++ ))
do
	makeAsteroidField `pos` `pos`
done

echo "Server launched."
./run
