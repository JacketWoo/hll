#!/bin/awk -f
#this shell is to filter a redis command protocal, for example:
#*3
#$9
#PEXPIREAT
#$46
#DE01f9b8dd58d0e6244e4d937d5b184ce9_PC:20160421
#$13
#1462412628000

BEGIN {
  len = 0; 
  found_del = 0;
  pre = "";
  prepre = "";
}

{
  if (len > 0 && found_del == 1) { 
    print $0; 
    len = len - 1;
    prepre = pre;
    pre = $0;
    next; 
  } 
}

/^PEXPIREAT/ { 
  found_del = 1; 
  len = len - 2;
  print prepre;
  print pre;
  print $0;
  prepre = pre;
  pre = $0;
  next; 
}

{
  found_del = 0;
  prepre = pre;
  pre = $0;
} 

/^\*/{ 
  len = substr($0, 2)*2;
  next; 
}

END {
  print "done";
}
