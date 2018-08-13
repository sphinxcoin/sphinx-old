#!/usr/bin/env bash
FILE=/root/mn_count
WALLET=https://github.com/SphinxcoinLtd/SphinxcoinQT/releases/download/v1.9.9/Sphinxcoin-1.9.9-x86_64-linux-gnu.tar.gz
BOOTSTRAP=https://www.dropbox.com/s/xtyn9540qx1ntva/bootstrap.zip?dl=1
vpsip=$(hostname -I | awk '{print $1}')
mncount=$(cat $FILE)
declare CONFIRM_SUMMARY=Y
declare ANOTHERMN=N

clear
if [[ $(whoami) != 'root' ]]; then
	echo 'You must be root to use this script.'
	exit 1
fi

declare AGREE=Y
declare CONFIRM=Y

function ask_yn() {
	question="$1"
	# also return result to this var
	default_answer=${!2}
	while true; do
		
		
		read -p "$question [y/n]: " REPLY
		if [ -z "$REPLY" ]; then
			REPLY="$default"
		fi
		
		case "$REPLY" in
			Y*|y*)
				eval $2=Y
				return 0;;
			N*|n*)
				eval $2=N
				return 1;;
		esac
	done
}

function ask_another() {

	while true; do
	  clear
		if ask_yn 'Would you like to set up one more MN on this server?' 'CONFIRM'; then
 				xthmn
			else
				break
			fi
	 done
}

function xthmn() {
#create mnX user and copy bootstrap
	mncount=$(cat $FILE)
	echo $((mncount+1)) > $FILE 
	mncount=$(cat $FILE)
	adduser --disabled-password --gecos "" mn$mncount
	usermod -a -G sudo mn$mncount
	sudo cp -rf ~/.Sphinxcoin /home/mn$mncount/
	rm -rf /home/mn$mncount/.Sphinxcoin/Sphinxcoin.conf
	clear
		while true; do
			echo -e "Key In the port your want the SPHX MN$mncount to listen on (normally 9999) and  press[ENTER]: \n"
			while true; do
				read sphxprtmnx
					if [[ $sphxprtmnx = $sphxprtmn1 ]]; then
						echo 'Port number must be different then port that was aleady used. Try again'
					else 
						break				
					fi
			done
			echo -e "Key In the Masternode privatekey and press[ENTER]: \n"
			echo -e "This is the output from masternode genkey command: \n"
			read privkeymnx
			clear
			xthmn_summary
				if ask_yn 'Is this correct?' 'CONFIRM_SUMMARY'; then
				break
				fi
		done
	sudo echo -e "rpcuser=$usrnammn1\nrpcpassword=$usrpasmn1\nrpcallowip=127.0.0.1\nrpcport=2235$mncount\nport=$sphxprtmnx\nserver=1\nlisten=1\ndaemon=1\nlogtimestamps=1\nmnconflock=1\nmasternode=1\nmasternodeaddr=$vpsip:$sphxprtmnx\nmasternodeprivkey=$privkeymnx\n" > /home/mn$mncount/.Sphinxcoin/Sphinxcoin.conf
	sudo chown -R mn$mncount:mn$mncount /home/mn$mncount/.Sphinxcoin
	sudo ufw allow $sphxprtmnx	
	runuser -l mn$mncount -c 'sphinxcoind'
	echo "Please wait 60 sec"
	sleep 60
	runuser -l mn$mncount -c 'sphinxcoin-cli getinfo'
	echo "Note block number, it should increase on next screen"
	sleep 30
	runuser -l mn$mncount -c 'sphinxcoin-cli getinfo'
	sleep 10
}


function mn1_summary() {
	echo 'MN1 Configuraton Summary:'
	echo "rpcuser=$usrnammn1"
	echo "rpcpassword=$usrpasmn1"
	echo "rpcallowip=127.0.0.1"
	echo "rpcport=22350"
	echo "port=$sphxprtmn1"
	echo "server=1"
	echo "listen=1"
	echo "daemon=1"
	echo "logtimestamps=1"
	echo "mnconflock=1"
	echo "masternode=1"
	echo "masternodeaddr=$vpsip:$sphxprtmn1"
	echo "masternodeprivkey=$privkeymn1"

}

function xthmn_summary() {
	echo "MN$mncount Configuraton Summary:"
	echo "rpcuser=$usrnammn1"
	echo "rpcpassword=$usrpasmn1"
	echo "rpcallowip=127.0.0.1"
	echo "rpcport=2235$mncount"
	echo "port=$sphxprtmnx"
	echo "server=1"
	echo "listen=1"
	echo "daemon=1"
	echo "logtimestamps=1"
	echo "mnconflock=1"
	echo "masternode=1"
	echo "masternodeaddr=$vpsip:$sphxprtmnx"
	echo "masternodeprivkey=$privkeymnx"

}


if [ -f $FILE ];
then
   if ask_yn 'At least one masternode exist would you like to setup secondary?' 'AGREE'
   then
	ask_another
   fi
else
   echo "Configuring first masternode"
	echo "1" >> $FILE
#create swap			
	fallocate -l 3G /swapfile
	chmod 600 /swapfile
	mkswap /swapfile
	swapon /swapfile
	echo -e "/swapfile   none    swap    sw    0   0 \n" >> /etc/fstab
#update system and install required packages
	sudo apt-get update -y
	sudo apt-get upgrade -y
	sudo apt-get install fail2ban unzip -y
	sudo ufw allow OpenSSH
	sudo ufw allow 9999
	sudo ufw default deny incoming
	sudo ufw default allow outgoing
	sudo ufw --force enable
#download sphinxcoind and bootstrap
	cd /opt
	mkdir temp
	cd temp
	wget $WALLET
	tar -zxvf Sphinxcoin-1.9.9-x86_64-linux-gnu.tar.gz
	cp Sphinxcoin-1.9.9-x86_64-linux-gnu/sphinxcoind /usr/local/bin/
	cp Sphinxcoin-1.9.9-x86_64-linux-gnu/sphinxcoin-cli /usr/local/bin/
	sphinxcoind  > /dev/null 2>&1
	wget $BOOTSTRAP -O boot.zip
	sudo unzip -o /opt/temp/boot.zip -d ~/.Sphinxcoin/
#setup on root rpc access
	clear
	echo -e "WARNING, this does not really matter what you put here, just make it random\n\n"
	echo -e "Key In a random User Name for RPC access (you dont need to use it anywhere) and press [ENTER]: \n"
	read usrnam
	echo -e "Key In a LONG RANDOM PASSWORD for the above user (you dont need to use it anywhere) and press[ENTER]: \n"
	read usrpas
	echo -e "rpcuser=$usrnam\nrpcpassword=$usrpas\nrpcallowip=127.0.0.1\nrpcport=22350\nport=9999\nserver=1\nlisten=1\ndaemon=1\nlogtimestamps=1\nmnconflock=0\n" > ~/.Sphinxcoin/Sphinxcoin.conf
	sphinxcoind 
	echo "Wait 60 sec"
	sleep 60
	sphinxcoin-cli getinfo
	sleep 10
	sphinxcoin-cli stop
#create mn1 user and copy bootstrap
	adduser --disabled-password --gecos "" mn1
	usermod -a -G sudo mn1
	sudo cp -rf ~/.Sphinxcoin /home/mn1/
	rm -rf /home/mn1/.Sphinxcoin/Sphinxcoin.conf
	clear
		while true; do
			echo -e "Key In a User Name for MN access (same as in wallet) \n"
			echo -e "You can find it in Windows wallet PC under this location\n"
			echo -e "click start and type in %appdata%/Sphinxcoin and enter, edit Sphinxcoin.conf file\n"
			echo -e "Please key in rpcuser here and press [ENTER]:\n"
			read usrnammn1
			echo -e "Key In a MN PASSWORD (same as in your wallet) can be found in same file as before: \n"
			echo -e "Please key in rpcpassword here and press [ENTER]:\n"
			read usrpasmn1
			echo -e "Key In the port your want the SPHX MN to listen on (normally 9999) and  press[ENTER]: \n"
			read sphxprtmn1
			echo -e "Key In the Masternode privatekey and press[ENTER]: \n"
			echo -e "This is the output from masternode genkey command: \n"
			read privkeymn1
			clear
			mn1_summary
				if ask_yn 'Is this correct?' 'CONFIRM_SUMMARY'; then
				break
				fi
		done
	sudo echo -e "rpcuser=$usrnammn1\nrpcpassword=$usrpasmn1\nrpcallowip=127.0.0.1\nrpcport=22350\nport=$sphxprtmn1\nserver=1\nlisten=1\ndaemon=1\nlogtimestamps=1\nmnconflock=1\nmasternode=1\nmasternodeaddr=$vpsip:$sphxprtmn1\nmasternodeprivkey=$privkeymn1\n" > /home/mn1/.Sphinxcoin/Sphinxcoin.conf
	sudo chown -R mn1:mn1 /home/mn1/.Sphinxcoin
	sudo ufw allow $sphxprtmn1	
	runuser -l mn1 -c 'sphinxcoind'
	echo "Please wait 60 sec"
	sleep 60
	runuser -l mn1 -c 'sphinxcoin-cli getinfo'
	echo "Note block number, it should increase on next screen"
	sleep 30
	runuser -l mn1 -c 'sphinxcoin-cli getinfo'
	sleep 10
fi	
clear

ask_another

clear
echo -e "\nScript complete and masternode is synchonizing\n"
echo -e "Now execute following commands in this order\n"
echo -e "su mn1\nsphinxcoin-cli getinfo\n"
echo -e "Repeat until fully synced under each mn to current block\n"
echo -e "Press start masternode on your wallet PC for each\n"
echo -e "sphinxcoin-cli masternode status\n"
echo -e "Status code should be 9\n"

echo -e "\n Please consider donating to: Sdj95FWteZ9ArJw6tKq7yANRkdaZ2EB4nP "







