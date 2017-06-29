echo 1 > /proc/sys/net/ipv4/ip_forward
iptables -A FORWARD -i tun0 -o eth0 -j ACCEPT
iptables -A FORWARD -i eth0 -o tun0 -j ACCEPT
iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE