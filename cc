#!/bin/bash
#proveedor=$(curl -s https://www.whoismyisp.org | grep -oP -m1 '(?<=isp">).*(?=</p)')
#if [[ $proveedor == *Amazon* ]]; then
#sudo su
#fi
#if [[ $proveedor == *Microsoft* ]]; then
#sudo su
#fi
if [[ "$USER" != 'root' ]]; then
  echo "Este Script Solo Funciona Para Usuarios root"
  exit
fi
sistema_operativo=$(cat /etc/os-release)
MYIP=$(wget -qO- ipv4.icanhazip.com);
MYIP2="s/xxxxxxxxx/$MYIP/g";
Plugin_autent='';
if [ -f /usr/lib/x86_64-linux-gnu/openvpn/plugins/openvpn-plugin-auth-pam.so ]; then
Plugin_autent='/usr/lib/x86_64-linux-gnu/openvpn/plugins/openvpn-plugin-auth-pam.so';
else
Plugin_autent='/usr/lib/openvpn/openvpn-plugin-auth-pam.so';
fi

function ubuntu_14(){
echo "INSTALANDO OPENVPN EN UBUNTU 14...\n"
apt-get -y install openvpn easy-rsa openssl iptables > /dev/null 2>&1
cp -r /usr/share/easy-rsa/ /etc/openvpn
mkdir /etc/openvpn/easy-rsa/keys > /dev/null 2>&1
sed -i 's|export KEY_COUNTRY="mx"|export KEY_COUNTRY="mx"|' /etc/openvpn/easy-rsa/vars
sed -i 's|export KEY_PROVINCE="mx"|export KEY_PROVINCE="mx"|' /etc/openvpn/easy-rsa/vars
sed -i 's|export KEY_CITY="mx"|export KEY_CITY="mx"|' /etc/openvpn/easy-rsa/vars
sed -i 's|export KEY_ORG="mx"|export KEY_ORG="mx"|' /etc/openvpn/easy-rsa/vars
sed -i 's|export KEY_EMAIL="@mx"|export KEY_EMAIL="@mx"|' /etc/openvpn/easy-rsa/vars
sed -i 's|export KEY_OU="mx"|export KEY_OU="mx"|' /etc/openvpn/easy-rsa/vars
sed -i 's|export KEY_NAME="cod3err0r"|export KEY_NAME="cod3err0r"|' /etc/openvpn/easy-rsa/vars
sed -i 's|export KEY_OU=cod3err0r|export KEY_OU=cod3err0r|' /etc/openvpn/easy-rsa/vars
# Create Diffie-Helman Pem
openssl dhparam -out /etc/openvpn/dh2048.pem 2048 > /dev/null 2>&1
# Create PKI
cd /etc/openvpn/easy-rsa
. ./vars
./clean-all
export EASY_RSA="${EASY_RSA:-.}"
"$EASY_RSA/pkitool" --initca $*
# Create key server
export EASY_RSA="${EASY_RSA:-.}"
"$EASY_RSA/pkitool" --server server
# Setting KEY CN
export EASY_RSA="${EASY_RSA:-.}"
"$EASY_RSA/pkitool" client
# cp /etc/openvpn/easy-rsa/keys/{server.crt,server.key,ca.crt} /etc/openvpn
cd
cp /etc/openvpn/easy-rsa/keys/server.crt /etc/openvpn/server.crt
cp /etc/openvpn/easy-rsa/keys/server.key /etc/openvpn/server.key
cp /etc/openvpn/easy-rsa/keys/ca.crt /etc/openvpn/ca.crt
# Setting Server
cd /etc/openvpn/
rm server.conf > /dev/null 2>&1
wget "https://github.com/egrojlive/codeerror/raw/master/server.conf" > /dev/null 2>&1
#Create OpenVPN Config
cd
rm client.ovpn > /dev/null 2>&1
wget "https://github.com/egrojlive/codeerror/raw/master/client.ovpn" > /dev/null 2>&1
cp client.ovpn clienttcp.ovpn
sed -i $MYIP2 clienttcp.ovpn;
echo '<ca>' >> clienttcp.ovpn
cat /etc/openvpn/ca.crt >> clienttcp.ovpn
echo '</ca>' >> clienttcp.ovpn
# Restart OpenVPN
/etc/init.d/openvpn restart > /dev/null 2>&1
service openvpn start > /dev/null 2>&1
service openvpn status
# Setting USW
apt-get install ufw -y > /dev/null 2>&1
ufw allow ssh
ufw allow 80/tcp
ufw allow 81/tcp
ufw allow 3128/tcp
ufw allow 8080/tcp
ufw allow 107/tcp
ufw allow 108/tcp
ufw allow 109/tcp
ufw allow 110/tcp
ufw allow 111/tcp
ufw allow 442/tcp
ufw allow 443/tcp
ufw allow 666/tcp
ufw allow 8181/tcp
ufw allow 4444/tcp
sed -i 's|DEFAULT_INPUT_POLICY="DROP"|DEFAULT_INPUT_POLICY="ACCEPT"|' /etc/default/ufw
sed -i 's|DEFAULT_FORWARD_POLICY="DROP"|DEFAULT_FORWARD_POLICY="ACCEPT"|' /etc/default/ufw
cd /etc/ufw/
rm beore.rules > /dev/null 2>&1
wget "https://github.com/egrojlive/codeerror/raw/master/before.rules" > /dev/null 2>&1
cd
DEBIAN_FRONTEND=noninteractive | echo "y" | ufw enable > /dev/null 2>&1
ufw status
#ufw disable
# set ipv4 forward
echo 1 > /proc/sys/net/ipv4/ip_forward
sed -i 's|#net.ipv4.ip_forward=1|net.ipv4.ip_forward=1|' /etc/sysctl.conf
echo "PROCESO TERMINADO CORREACTAMENTE"
}
function ubuntu(){
echo "INSTALANDO OPENVPN EN UBUNTU 16+...\n"
echo "VERIFICANDO REQUERIMIENTOS"
TCP_SERVICE_AND_CONFIG_NAME="openvpn_tcp"
UDP_SERVICE_AND_CONFIG_NAME="openvpn_udp"
###############################################################
if [[ "$USER" != 'root' ]]; then
  echo "LO SENTIMOS ESTE SCRIPT SOLO SE PUEDE EJECUTAR COMO ROOT"
  exit
fi
###############################################################
if [[ ! -e /dev/net/tun ]]; then
  echo "TUN/TAP is not available"
  exit
fi
###############################################################
if grep -qs "CentOS release 5" "/etc/redhat-release"; then
  echo "CentOS 5 NO ES SOPORTADO"
  exit
fi
###############################################################
if [[ -e /etc/debian_version ]]; then
  OS=debian
  RCLOCAL='/etc/rc.local'
elif [[ -e /etc/centos-release || -e /etc/redhat-release ]]; then
  OS=centos
  RCLOCAL='/etc/rc.d/rc.local'
  # Needed for CentOS 7
  chmod +x /etc/rc.d/rc.local
else
  echo "ESTE SCRIPT SOLO FUNCIONA EN : Debian Y Ubuntu"
  exit
fi
###############################################################
newclienttcp () {
  # This function is used to create tcp client .ovpn file
  cp /etc/openvpn/clienttcp-common.txt ~/"$1tcp.ovpn"
  echo "<ca>" >> ~/"$1tcp.ovpn"
  cat /etc/openvpn/easy-rsa/pki/ca.crt >> ~/"$1tcp.ovpn"
  echo "</ca>" >> ~/"$1tcp.ovpn"
  echo "<cert>" >> ~/"$1tcp.ovpn"
  cat /etc/openvpn/easy-rsa/pki/issued/"$1.crt" >> ~/"$1tcp.ovpn"
  echo "</cert>" >> ~/"$1tcp.ovpn"
  echo "<key>" >> ~/"$1tcp.ovpn"
  cat /etc/openvpn/easy-rsa/pki/private/"$1.key" >> ~/"$1tcp.ovpn"
  echo "</key>" >> ~/"$1tcp.ovpn"
  if [ "$TLS" = "1" ]; then  #check if TLS is selected to add a TLS static key
    echo "key-direction 1" >> ~/"$1tcp.ovpn"
    echo "<tls-auth>" >> ~/"$1tcp.ovpn"
    cat /etc/openvpn/easy-rsa/pki/private/ta.key >> ~/"$1tcp.ovpn"
    echo "</tls-auth>" >> ~/"$1tcp.ovpn"
  fi
  if [ $TLSNEW = 1 ]; then
    echo "--tls-version-min 1.2" >> ~/"$1.ovpn"
  fi
}
###############################################################
function version_gt() {
   test "$(echo "$@" | tr " " "\n" | sort -V | head -n 1)" != "$1";
    }
###############################################################
    IP=$(wget -qO- ipv4.icanhazip.com)
###############################################################
  clear
  echo "listening to. " $IP
  #read -p "IP address: " -e -i $IP IP
  echo ""
  #read -p "Do you want to run a UDP server [y/n]: " -e -i y UDP
  TCP=1
  #read -p "Do you want to run a TCP server [y/n]: " -e -i n TCP
  ###############################################################
  clear
  #read -p "What UDP port do you want to run OpenVPN on?: " -e -i 1194 PORT
  echo "PUERTO DE ESCUCHA : 443"
  PORTTCP=443
  #read -p "What TCP port do you want to run OpenVPN on?: " -e -i 443 PORTTCP
   echo "USANDO KEY 2048 BITS"
  KEYSIZE=2048
  DIGEST=SHA256
  #read -p "Digest Size [1-2]: " -e -i 1 DIGEST
###############################################################
  AES=0
        grep -q aes /proc/cpuinfo #Check for AES-NI availability
        if [[ "$?" -eq 0 ]]; then
         AES=1
        fi

   if [[ "$AES" -eq 1 ]]; then
         echo "Your CPU supports AES-NI instruction set."
   fi
echo "USANDO CIRADO : AES-256-CBC"
CIPHER=AES-256-CBC
echo "USANDO TLS-AUTH"
TLS=1
#read -p "Do you want to use additional TLS authentication [y/n]: " -e -i y TLS
INTERNALNETWORK=1
echo "USANDO DNS 1.1.1.1 - 9.9.9.9"
#read -p "Allow internal networking [y/n]: " -e -i y INTERNALNETWORK
DNSRESOLVER=0
ANTIVIR=0
###############################################################
if [ "$DNSRESOLVER" = 0 ]; then
DNS=1
#read -p "DNS [1-6]: " -e -i 1 DNS
CLIENT='client'
#read -p "Client name: " -e -i client CLIENT
if [[ "$OS" = 'debian' ]]; then


apt-get update -qq -y > /dev/null 2>&1
apt-get install openvpn iptables openssl -y -qq > /dev/null 2>&1
apt-get install build-essential libssl-dev liblzo2-dev libpam0g-dev easy-rsa -y > /dev/null 2>&1
ovpnversion=$(openvpn --status-version | grep -o "([0-9].*)" | sed 's/[^0-9.]//g')
if version_gt $ovpnversion "2.3.3"; then
echo "Your OpenVPN version is $ovpnversion and it supports"
echo "NOTE: Your client also must use version 2.3.3 or newer"
TLSNEW=1
#read -p "Force TLS 1.2 [y/n]: " -e -i n TLSNEW
fi

###############################################################
if [[ -d /etc/openvpn/easy-rsa/ ]]; then
  rm -rf /etc/openvpn/easy-rsa/
fi
# Get easy-rsa
wget --no-check-certificate -O ~/EasyRSA-3.0.1.tgz https://github.com/OpenVPN/easy-rsa/releases/download/3.0.1/EasyRSA-3.0.1.tgz > /dev/null 2>&1
tar xzf ~/EasyRSA-3.0.1.tgz -C ~/
mkdir /etc/openvpn
mv ~/EasyRSA-3.0.1/ /etc/openvpn/EasyRSA-3.0.1
mv /etc/openvpn/EasyRSA-3.0.1/ /etc/openvpn/easy-rsa/
chown -R root:root /etc/openvpn/easy-rsa/
rm -rf ~/EasyRSA-3.0.1.tgz
cd /etc/openvpn/easy-rsa/
# Create the PKI, set up the CA, the DH params and the server + client certificates
./easyrsa init-pki
cp vars.example vars
sed -i 's/#set_var EASYRSA_KEY_SIZE 2048/set_var EASYRSA_KEY_SIZE   '$KEYSIZE'/' vars
./easyrsa --batch build-ca nopass
./easyrsa gen-dh
./easyrsa build-server-full server nopass
./easyrsa build-client-full "$CLIENT" nopass
./easyrsa gen-crl
openvpn --genkey --secret /etc/openvpn/easy-rsa/pki/private/ta.key
cp pki/ca.crt pki/private/ca.key pki/dh.pem pki/issued/server.crt pki/private/server.key /etc/openvpn
echo "GENERANDO CERTIFICADO"
echo "port $PORTTCP
proto tcp
dev tun
ca ca.crt
cert server.crt
key server.key
dh dh.pem
push \"register-dns\"
topology subnet
server 10.9.0.0 255.255.255.0
ifconfig-pool-persist ipp.txt
cipher AES-256-CBC
user nobody
group nogroup
client-cert-not-required
username-as-common-name
plugin $Plugin_autent login
sndbuf 0
rcvbuf 0
push \"redirect-gateway def1 bypass-dhcp\"
--tls-auth /etc/openvpn/easy-rsa/pki/private/ta.key 0
push \"dhcp-option DNS 1.1.1.1\"
push \"dhcp-option DNS 9.9.9.9\"
keepalive 10 120
comp-lzo
persist-key
persist-tun
status openvpn-status.log
verb 3
crl-verify /etc/openvpn/easy-rsa/pki/crl.pem
client-to-client
" > /etc/openvpn/$TCP_SERVICE_AND_CONFIG_NAME.conf

  sed -i 's|#net.ipv4.ip_forward=1|net.ipv4.ip_forward=1|' /etc/sysctl.conf
  sed -i " 5 a\echo 1 > /proc/sys/net/ipv4/ip_forward" $RCLOCAL    # Added for servers that don't read from sysctl at startup

  echo 1 > /proc/sys/net/ipv4/ip_forward
  # Set NAT for the VPN subnet
     if [ "$INTERNALNETWORK" = 1 ]; then
    if [ "$TCP" = 1 ]; then
      iptables -t nat -A POSTROUTING -s 10.9.0.0/24 ! -d 10.9.0.0/24 -j SNAT --to $IP
      sed -i "1 a\iptables -t nat -A POSTROUTING -s 10.9.0.0/24 ! -d 10.9.0.0/24 -j SNAT --to $IP" $RCLOCAL
      fi
     else
    if [ "$TCP" = 1 ]; then
      iptables -t nat -A POSTROUTING -s 10.9.0.0/24  ! -d 10.9.0.1 -j SNAT --to $IP #This line and the next one are added for tcp server instance
      sed -i "1 a\iptables -t nat -A POSTROUTING -s 10.9.0.0/24 -j SNAT --to $IP" $RCLOCAL
    fi
   fi

  if iptables -L | grep -q REJECT; then
    if [ "$TCP" = 1 ]; then
      iptables -I INPUT -p udp --dport $PORTTCP -j ACCEPT #This line and next 5 lines have been added for tcp support
      iptables -I FORWARD -s 10.9.0.0/24 -j ACCEPT
      iptables -I FORWARD -m state --state RELATED,ESTABLISHED -j ACCEPT
      sed -i "1 a\iptables -I INPUT -p tcp --dport $PORTTCP -j ACCEPT" $RCLOCAL
      sed -i "1 a\iptables -I FORWARD -s 10.9.0.0/24 -j ACCEPT" $RCLOCAL
      sed -i "1 a\iptables -I FORWARD -m state --state RELATED,ESTABLISHED -j ACCEPT" $RCLOCAL
    fi
  fi

  if [ "$TCP" = 1 ]; then
    echo "[Unit]
#Created by openvpn-install-advanced (https://github.com/pl48415/openvpn-install-advanced)
Description=OpenVPN Robust And Highly Flexible Tunneling Application On <server>
After=syslog.target network.target

[Service]
Type=forking
PIDFile=/var/run/openvpn/$TCP_SERVICE_AND_CONFIG_NAME.pid
ExecStart=/usr/sbin/openvpn --daemon --writepid /var/run/openvpn/$TCP_SERVICE_AND_CONFIG_NAME.pid --cd /etc/openvpn/ --config $TCP_SERVICE_AND_CONFIG_NAME.conf

[Install]
WantedBy=multi-user.target" > /etc/systemd/system/$TCP_SERVICE_AND_CONFIG_NAME.service
    if pgrep systemd-journal; then
      sudo systemctl enable $TCP_SERVICE_AND_CONFIG_NAME.service
    fi
  fi

  if pgrep systemd-journal; then
    sudo systemctl start openvpn.service
  else
    if [[ "$OS" = 'debian' ]]; then
      /etc/init.d/openvpn start
    else
      service openvpn start
    fi
  fi
service openvpn_tcp restart
  EXTERNALIP=$(wget -qO- ipv4.icanhazip.com)
if [ "$TCP" = 1 ]; then
echo "client
cipher $CIPHER
auth-user-pass
dev tun
proto tcp
remote $IP $PORTTCP tcp-client
resolv-retry infinite
nobind
persist-key
persist-tun
remote-cert-tls server
comp-lzo
verb 3
sndbuf 0
rcvbuf 0
" > /etc/openvpn/clienttcp-common.txt
newclienttcp "$CLIENT"
  fi
  if [ "$TCP" = 1 ]; then
  echo "Your TCP client config is available at ~/${CLIENT}tcp.ovpn"
  fi
fi
if [ "$DNSRESOLVER" = 1 ]; then
service unbound restart
service openvpn_tcp restart
fi
fi
service openvpn_tcp restart
  }
function debian(){
echo "INSTALANDO OPENVPN DEBIAN...\n"
echo "VERIFICANDO REQUERIMIENTOS"
TCP_SERVICE_AND_CONFIG_NAME="openvpn_tcp"
UDP_SERVICE_AND_CONFIG_NAME="openvpn_udp"
###############################################################
if [[ "$USER" != 'root' ]]; then
  echo "LO SENTIMOS ESTE SCRIPT SOLO SE PUEDE EJECUTAR COMO ROOT"
  exit
fi
###############################################################
if [[ ! -e /dev/net/tun ]]; then
  echo "TUN/TAP is not available"
  exit
fi
###############################################################
if grep -qs "CentOS release 5" "/etc/redhat-release"; then
  echo "CentOS 5 NO ES SOPORTADO"
  exit
fi
###############################################################
if [[ -e /etc/debian_version ]]; then
  OS=debian
  RCLOCAL='/etc/rc.local'
elif [[ -e /etc/centos-release || -e /etc/redhat-release ]]; then
  OS=centos
  RCLOCAL='/etc/rc.d/rc.local'
  # Needed for CentOS 7
  chmod +x /etc/rc.d/rc.local
else
  echo "ESTE SCRIPT SOLO FUNCIONA EN : Debian Y Ubuntu"
  exit
fi
###############################################################
newclienttcp () {
  # This function is used to create tcp client .ovpn file
  cp /etc/openvpn/clienttcp-common.txt ~/"$1tcp.ovpn"
  echo "<ca>" >> ~/"$1tcp.ovpn"
  cat /etc/openvpn/easy-rsa/pki/ca.crt >> ~/"$1tcp.ovpn"
  echo "</ca>" >> ~/"$1tcp.ovpn"
  echo "<cert>" >> ~/"$1tcp.ovpn"
  cat /etc/openvpn/easy-rsa/pki/issued/"$1.crt" >> ~/"$1tcp.ovpn"
  echo "</cert>" >> ~/"$1tcp.ovpn"
  echo "<key>" >> ~/"$1tcp.ovpn"
  cat /etc/openvpn/easy-rsa/pki/private/"$1.key" >> ~/"$1tcp.ovpn"
  echo "</key>" >> ~/"$1tcp.ovpn"
  if [ "$TLS" = "1" ]; then  #check if TLS is selected to add a TLS static key
    echo "key-direction 1" >> ~/"$1tcp.ovpn"
    echo "<tls-auth>" >> ~/"$1tcp.ovpn"
    cat /etc/openvpn/easy-rsa/pki/private/ta.key >> ~/"$1tcp.ovpn"
    echo "</tls-auth>" >> ~/"$1tcp.ovpn"
  fi
  if [ $TLSNEW = 1 ]; then
    echo "--tls-version-min 1.2" >> ~/"$1.ovpn"
  fi
}
###############################################################
function version_gt() {
   test "$(echo "$@" | tr " " "\n" | sort -V | head -n 1)" != "$1";
    }
###############################################################
    IP=$(wget -qO- ipv4.icanhazip.com)
###############################################################
  clear
  echo "listening to. " $IP
  #read -p "IP address: " -e -i $IP IP
  echo ""
  #read -p "Do you want to run a UDP server [y/n]: " -e -i y UDP
  TCP=1
  #read -p "Do you want to run a TCP server [y/n]: " -e -i n TCP
  ###############################################################
  clear
  #read -p "What UDP port do you want to run OpenVPN on?: " -e -i 1194 PORT
  echo "PUERTO DE ESCUCHA : 4444"
  PORTTCP=4444
  #read -p "What TCP port do you want to run OpenVPN on?: " -e -i 443 PORTTCP
   echo "USANDO KEY 2048 BITS"
  KEYSIZE=2048
  DIGEST=SHA256
  #read -p "Digest Size [1-2]: " -e -i 1 DIGEST
###############################################################
  AES=0
        grep -q aes /proc/cpuinfo #Check for AES-NI availability
        if [[ "$?" -eq 0 ]]; then
         AES=1
        fi

   if [[ "$AES" -eq 1 ]]; then
         echo "Your CPU supports AES-NI instruction set."
   fi
echo "USANDO CIRADO : AES-256-CBC"
CIPHER=AES-256-CBC
echo "USANDO TLS-AUTH"
TLS=1
#read -p "Do you want to use additional TLS authentication [y/n]: " -e -i y TLS
INTERNALNETWORK=1
echo "USANDO DNS 1.1.1.1 - 9.9.9.9"
#read -p "Allow internal networking [y/n]: " -e -i y INTERNALNETWORK
DNSRESOLVER=0
ANTIVIR=0
###############################################################
if [ "$DNSRESOLVER" = 0 ]; then
DNS=1
#read -p "DNS [1-6]: " -e -i 1 DNS
CLIENT='client'
#read -p "Client name: " -e -i client CLIENT
if [[ "$OS" = 'debian' ]]; then


apt-get update -qq -y > /dev/null 2>&1
apt-get install openvpn iptables openssl -y -qq > /dev/null 2>&1
apt-get install build-essential libssl-dev liblzo2-dev libpam0g-dev easy-rsa -y > /dev/null 2>&1
ovpnversion=$(openvpn --status-version | grep -o "([0-9].*)" | sed 's/[^0-9.]//g')
if version_gt $ovpnversion "2.3.3"; then
echo "Your OpenVPN version is $ovpnversion and it supports"
echo "NOTE: Your client also must use version 2.3.3 or newer"
TLSNEW=1
#read -p "Force TLS 1.2 [y/n]: " -e -i n TLSNEW
fi

###############################################################
if [[ -d /etc/openvpn/easy-rsa/ ]]; then
  rm -rf /etc/openvpn/easy-rsa/
fi
# Get easy-rsa
wget --no-check-certificate -O ~/EasyRSA-3.0.1.tgz https://github.com/OpenVPN/easy-rsa/releases/download/3.0.1/EasyRSA-3.0.1.tgz > /dev/null 2>&1
tar xzf ~/EasyRSA-3.0.1.tgz -C ~/
mkdir /etc/openvpn
mv ~/EasyRSA-3.0.1/ /etc/openvpn/EasyRSA-3.0.1
mv /etc/openvpn/EasyRSA-3.0.1/ /etc/openvpn/easy-rsa/
chown -R root:root /etc/openvpn/easy-rsa/
rm -rf ~/EasyRSA-3.0.1.tgz
cd /etc/openvpn/easy-rsa/
# Create the PKI, set up the CA, the DH params and the server + client certificates
./easyrsa init-pki
cp vars.example vars
sed -i 's/#set_var EASYRSA_KEY_SIZE 2048/set_var EASYRSA_KEY_SIZE   '$KEYSIZE'/' vars
./easyrsa --batch build-ca nopass
./easyrsa gen-dh
./easyrsa build-server-full server nopass
./easyrsa build-client-full "$CLIENT" nopass
./easyrsa gen-crl
openvpn --genkey --secret /etc/openvpn/easy-rsa/pki/private/ta.key
cp pki/ca.crt pki/private/ca.key pki/dh.pem pki/issued/server.crt pki/private/server.key /etc/openvpn
echo "GENERANDO CERTIFICADO"
echo "port $PORTTCP
proto tcp
dev tun
ca ca.crt
cert server.crt
key server.key
dh dh.pem
push \"register-dns\"
topology subnet
server 10.9.0.0 255.255.255.0
ifconfig-pool-persist ipp.txt
cipher AES-256-CBC
user nobody
group nogroup
client-cert-not-required
username-as-common-name
plugin $Plugin_autent login
sndbuf 0
rcvbuf 0
push \"redirect-gateway def1 bypass-dhcp\"
--tls-auth /etc/openvpn/easy-rsa/pki/private/ta.key 0
push \"dhcp-option DNS 1.1.1.1\"
push \"dhcp-option DNS 9.9.9.9\"
keepalive 10 120
comp-lzo
persist-key
persist-tun
status openvpn-status.log
verb 3
crl-verify /etc/openvpn/easy-rsa/pki/crl.pem
client-to-client
" > /etc/openvpn/$TCP_SERVICE_AND_CONFIG_NAME.conf

  sed -i 's|#net.ipv4.ip_forward=1|net.ipv4.ip_forward=1|' /etc/sysctl.conf
  sed -i " 5 a\echo 1 > /proc/sys/net/ipv4/ip_forward" $RCLOCAL    # Added for servers that don't read from sysctl at startup

  echo 1 > /proc/sys/net/ipv4/ip_forward
  # Set NAT for the VPN subnet
     if [ "$INTERNALNETWORK" = 1 ]; then
    if [ "$TCP" = 1 ]; then
      iptables -t nat -A POSTROUTING -s 10.9.0.0/24 ! -d 10.9.0.0/24 -j SNAT --to $IP
      sed -i "1 a\iptables -t nat -A POSTROUTING -s 10.9.0.0/24 ! -d 10.9.0.0/24 -j SNAT --to $IP" $RCLOCAL
      fi
     else
    if [ "$TCP" = 1 ]; then
      iptables -t nat -A POSTROUTING -s 10.9.0.0/24  ! -d 10.9.0.1 -j SNAT --to $IP #This line and the next one are added for tcp server instance
      sed -i "1 a\iptables -t nat -A POSTROUTING -s 10.9.0.0/24 -j SNAT --to $IP" $RCLOCAL
    fi
   fi

  if iptables -L | grep -q REJECT; then
    if [ "$TCP" = 1 ]; then
      iptables -I INPUT -p udp --dport $PORTTCP -j ACCEPT #This line and next 5 lines have been added for tcp support
      iptables -I FORWARD -s 10.9.0.0/24 -j ACCEPT
      iptables -I FORWARD -m state --state RELATED,ESTABLISHED -j ACCEPT
      sed -i "1 a\iptables -I INPUT -p tcp --dport $PORTTCP -j ACCEPT" $RCLOCAL
      sed -i "1 a\iptables -I FORWARD -s 10.9.0.0/24 -j ACCEPT" $RCLOCAL
      sed -i "1 a\iptables -I FORWARD -m state --state RELATED,ESTABLISHED -j ACCEPT" $RCLOCAL
    fi
  fi

  if [ "$TCP" = 1 ]; then
    echo "[Unit]
#Created by openvpn-install-advanced (https://github.com/pl48415/openvpn-install-advanced)
Description=OpenVPN Robust And Highly Flexible Tunneling Application On <server>
After=syslog.target network.target

[Service]
Type=forking
PIDFile=/var/run/openvpn/$TCP_SERVICE_AND_CONFIG_NAME.pid
ExecStart=/usr/sbin/openvpn --daemon --writepid /var/run/openvpn/$TCP_SERVICE_AND_CONFIG_NAME.pid --cd /etc/openvpn/ --config $TCP_SERVICE_AND_CONFIG_NAME.conf

[Install]
WantedBy=multi-user.target" > /etc/systemd/system/$TCP_SERVICE_AND_CONFIG_NAME.service
    if pgrep systemd-journal; then
      sudo systemctl enable $TCP_SERVICE_AND_CONFIG_NAME.service
    fi
  fi

  if pgrep systemd-journal; then
    sudo systemctl start openvpn.service
  else
    if [[ "$OS" = 'debian' ]]; then
      /etc/init.d/openvpn start
    else
      service openvpn start
    fi
  fi
service openvpn_tcp restart
  EXTERNALIP=$(wget -qO- ipv4.icanhazip.com)
if [ "$TCP" = 1 ]; then
echo "client
cipher $CIPHER
auth-user-pass
dev tun
proto tcp
remote $IP $PORTTCP tcp-client
resolv-retry infinite
nobind
persist-key
persist-tun
remote-cert-tls server
comp-lzo
verb 3
sndbuf 0
rcvbuf 0
" > /etc/openvpn/clienttcp-common.txt
newclienttcp "$CLIENT"
  fi
  if [ "$TCP" = 1 ]; then
  echo "Your TCP client config is available at ~/${CLIENT}tcp.ovpn"
  fi
fi
if [ "$DNSRESOLVER" = 1 ]; then
service unbound restart
service openvpn_tcp restart
fi
fi
service openvpn_tcp restart
  }

if cat /etc/*release | grep DISTRIB_DESCRIPTION | grep "Ubuntu 14.04" > /dev/null 2>&1; then
ubuntu_14
elif [[ $sistema_operativo == *Ubuntu* ]]; then
ubuntu
elif [[ $sistema_operativo == *Debian* ]]; then
debian
else
echo "este script no es compatible con este sistema operativo\n" $sistema_operativo
fi


cd
	apt-get -y install nginx
	cat > /etc/nginx/nginx.conf <<END
user www-data;
worker_processes 2;
pid /var/run/nginx.pid;
events {
	multi_accept on;
        worker_connections 1024;
}
http {
	autoindex on;
        sendfile on;
        tcp_nopush on;
        tcp_nodelay on;
        keepalive_timeout 65;
        types_hash_max_size 2048;
        server_tokens off;
        include /etc/nginx/mime.types;
        default_type application/octet-stream;
        access_log /var/log/nginx/access.log;
        error_log /var/log/nginx/error.log;
        client_max_body_size 32M;
	client_header_buffer_size 8m;
	large_client_header_buffers 8 8m;
	fastcgi_buffer_size 8m;
	fastcgi_buffers 8 8m;
	fastcgi_read_timeout 600;
        include /etc/nginx/conf.d/*.conf;
}
END
	mkdir -p /home/vps/public_html
	echo "<pre>by phutthasit2530 | phutthasit2530</pre>" > /home/vps/public_html/index.html
	echo "<?phpinfo(); ?>" > /home/vps/public_html/info.php
	args='$args'
	uri='$uri'
	document_root='$document_root'
	fastcgi_script_name='$fastcgi_script_name'
	cat > /etc/nginx/conf.d/vps.conf <<END
server {
    listen       85;
    server_name  127.0.0.1 localhost;
    access_log /var/log/nginx/vps-access.log;
    error_log /var/log/nginx/vps-error.log error;
    root   /home/vps/public_html;
    location / {
        index  index.html index.htm index.php;
	try_files $uri $uri/ /index.php?$args;
    }
    location ~ \.php$ {
        include /etc/nginx/fastcgi_params;
        fastcgi_pass  127.0.0.1:9000;
        fastcgi_index index.php;
        fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
    }
}
END

	if [[ "$VERSION_ID" = 'VERSION_ID="7"' || "$VERSION_ID" = 'VERSION_ID="8"' || "$VERSION_ID" = 'VERSION_ID="14.04"' ]]; then
		if [[ -e /etc/squid3/squid.conf ]]; then
			apt-get -y remove --purge squid3
		fi

		apt-get -y install squid3
		cat > /etc/squid3/squid.conf <<END
http_port $PROXY
acl localhost src 127.0.0.1/32 ::1
acl to_localhost dst 127.0.0.0/8 0.0.0.0/32 ::1
acl localnet src 10.0.0.0/8
acl localnet src 172.16.0.0/12
acl localnet src 192.168.0.0/16
acl SSL_ports port 443
acl Safe_ports port 80
acl Safe_ports port 21
acl Safe_ports port 443
acl Safe_ports port 70
acl Safe_ports port 210
acl Safe_ports port 1025-65535
acl Safe_ports port 280
acl Safe_ports port 488
acl Safe_ports port 591
acl Safe_ports port 777
acl CONNECT method CONNECT
acl SSH dst xxxxxxxxx-xxxxxxxxx/255.255.255.255
http_access allow SSH
http_access allow localnet
http_access allow localhost
http_access deny all
refresh_pattern ^ftp:           1440    20%     10080
refresh_pattern ^gopher:        1440    0%      1440
refresh_pattern -i (/cgi-bin/|\?) 0     0%      0
refresh_pattern .               0       20%     4320
END
		IP2="s/xxxxxxxxx/$IP/g";
		sed -i $IP2 /etc/squid3/squid.conf;
		if [[ "$VERSION_ID" = 'VERSION_ID="14.04"' ]]; then
			service squid3 restart
			/etc/init.d/openvpn restart
			/etc/init.d/nginx restart
		else
			/etc/init.d/squid3 restart
			/etc/init.d/openvpn restart
			/etc/init.d/nginx restart
		fi

	elif [[ "$VERSION_ID" = 'VERSION_ID="9"' || "$VERSION_ID" = 'VERSION_ID="16.04"' || "$VERSION_ID" = 'VERSION_ID="18.04"' ]]; then
		if [[ -e /etc/squid/squid.conf ]]; then
			apt-get -y remove --purge squid
		fi

		apt-get -y install squid
		cat > /etc/squid/squid.conf <<END
http_port $PROXY
acl localhost src 127.0.0.1/32 ::1
acl to_localhost dst 127.0.0.0/8 0.0.0.0/32 ::1
acl localnet src 10.0.0.0/8
acl localnet src 172.16.0.0/12
acl localnet src 192.168.0.0/16
acl SSL_ports port 443
acl Safe_ports port 80
acl Safe_ports port 21
acl Safe_ports port 443
acl Safe_ports port 70
acl Safe_ports port 210
acl Safe_ports port 1025-65535
acl Safe_ports port 280
acl Safe_ports port 488
acl Safe_ports port 591
acl Safe_ports port 777
acl CONNECT method CONNECT
acl SSH dst xxxxxxxxx-xxxxxxxxx/255.255.255.255
http_access allow SSH
http_access allow localnet
http_access allow localhost
http_access deny all
refresh_pattern ^ftp:           1440    20%     10080
refsh_pattern ^gopher:        1440    0%      1440
refresh_pattern -i (/cgi-bin/|\?) 0     0%      0
refresh_pattern .               0       20%     4320
END
		IP2="s/xxxxxxxxx/$IP/g";
		sed -i $IP2 /etc/squid/squid.conf;
		/etc/init.d/squid restart
		/etc/init.d/openvpn restart
		/etc/init.d/nginx restart
	fi

fi


echo ""
echo -e "\033[0;32m { DOWNLOAD MENU SCRIPT }${NC} "
echo ""
	cd /usr/local/bin
wget -q -O m "https://raw.githubusercontent.com/phutthasit2530/openvpnauto/master/Menu"
chmod +x /usr/local/bin/m
	wget -O /usr/local/bin/Auto-Delete-Client "https://raw.githubusercontent.com/phutthasit2530/PURE/master/Auto-Delete-Client"
	chmod +x /usr/local/bin/Auto-Delete-Client 
	apt-get -y install vnstat
	cd /etc/openvpn/easy-rsa/
	./easyrsa build-client-full $CLIENT nopass
	newclient "$CLIENT"
	cp /root/$CLIENT.ovpn /home/vps/public_html/
	rm -f /root/$CLIENT.ovpn
	case $OPENVPNSYSTEM in
		2)
		useradd $Usernames
		echo -e "$Passwords\n$Passwords\n"|passwd $Usernames &> /dev/null
		;;
	esac
	
	
	clear
echo ""
echo ""
echo -e "${RED} =============== OS-32 & 64-bit =================    "
echo -e "${RED} #        AUTOSCRIPT CREATED BY ─━═हຫມາສີ້ແມ່ມືງह═━─         #    "
echo -e "${RED} #      -----------About Us------------         #    "
echo -e "${RED} #    OS  DEBIAN 7-8-9  OS  UBUNTU 14-16-18     #    "
echo -e "${RED} #       Truemoney Wallet : ─━═हຫມາສີ້ແມ່ມືງह═━─        #    "
echo -e "${RED} #               { VPN / SSH }                  #    "
echo -e "${RED} #         BY : ─━═हຫມາສີ້ແມ່ມືງह═━─               #    "
echo -e "${RED} #    FB : ─━═हຫມາສີ້ແມ່ມືງह═━─       #    "
echo -e "${RED} =============== OS-32 & 64-bit =================    "
echo -e "${GREEN} ไอพีเซิฟ: $IP "
echo -e "${NC} "
	echo "OpenVPN, Squid Proxy, Nginx .....Install finish."
	echo "IP Server : $IP"
	echo "Port Server : $PORT"
	if [[ "$PROTOCOL" = 'udp' ]]; then
		echo "Protocal : UDP"
	elif [[ "$PROTOCOL" = 'tcp' ]]; then
		echo "Protocal : TCP"
	fi
	echo "Port Nginx : 85"
	echo "IP Proxy : $IP"
	echo "Port Proxy : $PROXY"
	echo ""
	case $OPENVPNSYSTEM in
		1)
		echo "Download My Config : http://$IP:85/$CLIENT.ovpn"
		;;
		2)
		echo "Download Config : http://$IP:85/$CLIENT.ovpn"
		echo ""
		echo "Your Username : $Usernames"
		echo "Your Password : $Passwords"
		echo "Expire : Never"
		;;
		3)
		echo "Download Config : http://$IP:85/$CLIENT.ovpn"
		;;
	esac
	echo ""
	echo "===================================================================="
	echo -e "ติดตั้งสำเร็จ... กรุณาพิมพ์คำสั่ง${YELLOW} m ${NC} เพื่อไปยังขั้นตอนถัดไป"
	echo "===================================================================="
	echo ""
	exit


