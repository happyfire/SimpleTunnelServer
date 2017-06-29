echo 1 > /proc/sys/net/ipv4/ip_forward
iptables -A FORWARD -i xtun -o eth0 -j ACCEPT
iptables -A FORWARD -i eth0 -o xtun -j ACCEPT
iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE