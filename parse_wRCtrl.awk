# Author: Pavlo Nykolyn
# parses fields contained within the string:
# <IPv4>;[<port>];<model>
# <model> is the web relay model;
NF != 3 { print "err:1" }
# some awk versions may not support the fixed amount of matched characters
$1 !~ /^[0-9]{1,3}(\.[0-9]{1,3}){3}$/ { print "err:2" }
$2 !~ /^[0-9]*$/ { print "err:3" }
$2 > 65535 { print "err:3" }
$3 !~ /KMTronic_wr/ && $3 !~ /NC800/ { print "err:4" }
