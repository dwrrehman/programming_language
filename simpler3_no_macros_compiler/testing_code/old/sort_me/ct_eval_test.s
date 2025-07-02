( 1202504141.223536 dwrr  
an example i found on the wikipedia page for constant propagation:

  int a = 30;
  int b = 9 - (a / 5);
  int c = b * 4;  
  if (c > 10) {
     c = c - 10;
  }
  return c * (60 / a);
)

set -1 0 sub -1 1
set a 01111
set b a div b 101 sub b 1001 mul b -1
set c b mul c 001
ge 0101 c skip sub c 0101 at skip
set r 001111 div r a mul r c
rt return 0 set return r









