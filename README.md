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
./rate_test -l 1500 -t 1000 -c 100  -d "A2:15:1E:DE:A8:F7" -i enp2s0
rx info:
	lost pkts:	87523
	rx bytes:	454504314
	rx times:	5000043 us
	rx rate:	693 Mbps
	rx pkt sn:	2711991
	packet loss:	22.57353%
	rx pkts:	300201
	rx_rate/(1-packet_lost):	895 Mbps

tx info:
	tx bytes:	587082266
	tx times:	5000000 us
	tx rate:	895 Mbps
	tx pkt sn:	2712869
	tx pkts:	387769

sudo ./rate_test -l 1500 -t 1000 -c 80  -d "A2:15:1E:DE:A8:F7" -i enp2s0
rx info:
	lost pkts:	37849
	rx bytes:	452875250
	rx times:	5000072 us
	rx rate:	691 Mbps
	rx pkt sn:	1019112
	packet loss:	11.23202%
	rx pkts:	299125
	rx_rate/(1-packet_lost):	778 Mbps

tx info:
	tx bytes:	510278560
	tx times:	5001057 us
	tx rate:	778 Mbps
	tx pkt sn:	1019760
	tx pkts:	337040


sudo ./rate_test -l 1500 -t 1000 -c 75  -d "A2:15:1E:DE:A8:F7" -i enp2s0
rx info:
	lost pkts:	18922
	rx bytes:	452295388
	rx times:	5000008 us
	rx rate:	690 Mbps
	rx pkt sn:	1271137
	packet loss:	5.95661%
	rx pkts:	298742
	rx_rate/(1-packet_lost):	734 Mbps

tx info:
	tx bytes:	480997800
	tx times:	5000526 us
	tx rate:	733 Mbps
	tx pkt sn:	1271700
	tx pkts:	317700

sudo ./rate_test -l 1500 -t 1000 -c 70  -d "A2:15:1E:DE:A8:F7" -i enp2s0
rx info:
	lost pkts:	3822
	rx bytes:	450991834
	rx times:	5000063 us
	rx rate:	688 Mbps
	rx pkt sn:	901200
	packet loss:	1.26681%
	rx pkts:	297881
	rx_rate/(1-packet_lost):	697 Mbps

tx info:
	tx bytes:	456137920
	tx times:	5000997 us
	tx rate:	696 Mbps
	tx pkt sn:	901320
	tx pkts:	301280

sudo ./rate_test -l 1500 -t 1000 -c 65  -d "A2:15:1E:DE:A8:F7" -i enp2s0
tx info:
	tx bytes:	424739074
	tx times:	5000354 us
	tx rate:	648 Mbps
	tx pkt sn:	280540
	tx pkts:	280541

rx info:
	lost pkts:	261
	rx bytes:	424346948
	rx times:	5000052 us
	rx rate:	647 Mbps
	rx pkt sn:	280542
	packet loss:	0.09303%
	rx pkts:	280282
	rx_rate/(1-packet_lost):	648 Mbps


````
