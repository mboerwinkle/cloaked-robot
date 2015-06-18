
#!/bin/bash
rm sectors/*_*
echo 'entity
9
9
2
160000
1024
end
entity
4
3
0
1120000
32000
end
entity
4
3
0
2240000
-32000
end
entity
4
3
0
15200
-1600000
end
entity
4
3
0
-11289600
320000
end
entity
4
3
0
64000
3200
end
entity
3
2
2
0
0
end' > sectors/0_0
echo "Server launched. Come look at how sexy this minor is!!";
./run
