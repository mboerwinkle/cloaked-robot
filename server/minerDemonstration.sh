
#!/bin/bash
rm sectors/*_*
echo 'entity
9
9
2
10000
64
end
entity
4
3
0
70000
2000
end
entity
4
3
0
140000
-2000
end
entity
4
3
0
950
-100000
end
entity
4
3
0
-705600
20000
end
entity
4
3
0
4000
200
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
