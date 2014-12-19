#!/bin/bash

#Bash is a bit weird. The syntax for making a function looks like there are no arguments, but actually every function has arbitrarily many args. Maybe we should move to, like, python or lisp or something for this scripting. IDK.
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
echo $(( $3 - 8000 )) >> sectors/0_0
echo 'end
entity
9
9' >> sectors/0_0
echo $1 >> sectors/0_0
echo $2 >> sectors/0_0
echo $(( $3 + 8000 )) >> sectors/0_0
echo end >> sectors/0_0
}

# * 128 would give the full range of a sector, but that's not dense enough
pos () {
	echo $(( ($RANDOM - 16384) * 70 ))
}

rm sectors/*_*

#This shuffle means that while there will be 7 of each team's colonies, there's no set order in which they will be generated.
for i in `shuf -i 1-14`
do
#Even 'i's result in team 1; odd 'i's, team 2.
	makeColony $(($i % 2 + 1)) `pos` `pos`
done

for (( i=0; i<15; i++ ))
do
	makeAsteroidField `pos` `pos`
done

echo "Server launched. 7 of each team's colonies, 15 random asteroid fields. Have at 'em."
./run