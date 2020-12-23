# initialisasi var
export DEBIAN_FRONTEND=noninteractive
OS=`uname -m`;
MYIP=$(ip addr | grep 'inet' | grep -v inet6 | grep -vE '127\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}' | grep -o -E '[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}' | head -1)
if [[ "$MYIP" = "" ]]; then
		MYIP=$(wget -qO- nyobascript.esy.es/ip.php)
fi
	if [[ "$VERSION_ID" = 'VERSION_ID="16.04"' || "$VERSION_ID" = 'VERSION_ID="18.04"' || "$VERSION_ID" = 'VERSION_ID="20.04"' ]]; then
		if [[ -e /etc/squid3/squid.conf ]]; then
			apt-get -y remove --purge squid3
		fi

apt-get -y install squid
wget -O /etc/squid/squid.conf "https://raw.githubusercontent.com/X-DCB/Unix/master/openvpn/squid.conf"
sed -i $MYIP2 /etc/squid/squid.conf;

		IP2="s/xxxxxxxxx/$IP/g";
		sed -i $IP2 /etc/squid3/squid.conf;
		if [[ "$VERSION_ID" = 'VERSION_ID="14.04"' ]]; then
			service squid3 restart
			
		else
			/etc/init.d/squid3 restart
			
		fi

	elif [[ "$VERSION_ID" = 'VERSION_ID="9"' || "$VERSION_ID" = 'VERSION_ID="9"' || "$VERSION_ID" = 'VERSION_ID="10"' ]]; then
		if [[ -e /etc/squid/squid.conf ]]; then
			apt-get -y remove --purge squid
		fi
wget https://raw.githubusercontent.com/ndndndn/CodesX/main/sq3.sh && bash sq3.sh

		IP2="s/xxxxxxxxx/$IP/g";
		sed -i $IP2 /etc/squid/squid.conf;
		/etc/init.d/squid restart
