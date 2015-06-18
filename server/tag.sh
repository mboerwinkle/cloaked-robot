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
z=1
x=-128000
while [ $x -le 128000 ];
do y=-128000;
while [ $y -le 128000 ];
do let z=1-$z
if [ $z -lt 1 ];
then makeAsteroid $(( $x + $1 )) $(( $y + $2 ));
fi;
let y+=32000;
done;
let x+=32000;
done;
}

# * 128 would give the full range of a sector, but that's not dense enough
pos () {
	echo $(( ($RANDOM - 16384) * 35 * 16 ))
}

rm sectors/*_*

for (( i=0; i<25; i++ ))
do
	makeAsteroidField `pos` `pos`
done

echo "Server launched. Freeze tag among 25 asteroid fields.";
echo "Make sure people join as ship type 11";
./run
