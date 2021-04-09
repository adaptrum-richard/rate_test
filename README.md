# rate_test
parameters
````
 -l packet lenght, 
-t interval, 
-c Number of packets sent each time, 
-d dest mac
-i local send packet interface name
````
test and output
````
sudo ./rate_test -l 100 -t 1000 -c 10  -d "92:39:49:24:0B:6E" -i enp2s0
packet lenght:	100
interval(us):	1000
counts:		10
dest mac:	92:39:49:24:0B:6E
src mac:	40:8D:5C:5F:4E:CB
ifindex:	2
run client
tx info:
	tx bytes:	9873654
	tx times:	10000202 us
	tx rate:	7 Mbps
	tx pkt sn:	86610

rx info:
	lost pkts:	0
	rx bytes:	9873654
	rx times:	10000247 us
	rx rate:	7 Mbps
	rx pkt sn:	86610
	packet loss:	 0%
	rx_rate/(1-packet_lost):	 7 Mbps

````

````
#:~/test$ sudo ./rate_test -l 1500 -t 0 -c 0  -d "92:39:49:24:0B:6E" -i enp2s0
packet lenght:	1500
interval(us):	0
counts:		0
dest mac:	92:39:49:24:0B:6E
src mac:	40:8D:5C:5F:4E:CB
ifindex:	2
run client
tx info:
	tx bytes:	1071680358
	tx times:	10000024 us
	tx rate:	817 Mbps
	tx pkt sn:	707846

rx info:
	lost pkts:	106048
	rx bytes:	910171380
	rx times:	10000005 us
	rx rate:	694 Mbps
	rx pkt sn:	707217
	packet loss:	15%
	rx_rate/(1-packet_lost):	816 Mbps
````
