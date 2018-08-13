#! /usr/bin/env python
from subprocess import Popen,PIPE,STDOUT
import collections
import os
import sys
import time
import math
import os
from urllib2 import urlopen

SERVER_IP = urlopen('http://ip.42.pl/raw').read()
BOOTSTRAP_URL = "https://github.com/SphinxcoinLtd/SphinxcoinQT/releases/download/v1.9.9/blockchain-20180212.zip"

DEFAULT_COLOR = "\x1b[0m"
PRIVATE_KEYS = []

def print_info(message):
    BLUE = '\033[94m'
    print(BLUE + "[*] " + str(message) + DEFAULT_COLOR)
    time.sleep(1)

def print_warning(message):
    YELLOW = '\033[93m'
    print(YELLOW + "[*] " + str(message) + DEFAULT_COLOR)
    time.sleep(1)

def print_error(message):
    RED = '\033[91m'
    print(RED + "[*] " + str(message) + DEFAULT_COLOR)
    time.sleep(1)

def get_terminal_size():
    import fcntl, termios, struct
    h, w, hp, wp = struct.unpack('HHHH',
        fcntl.ioctl(0, termios.TIOCGWINSZ,
        struct.pack('HHHH', 0, 0, 0, 0)))
    return w, h
    
def remove_lines(lines):
    CURSOR_UP_ONE = '\x1b[1A'
    ERASE_LINE = '\x1b[2K'
    for l in lines:
        sys.stdout.write(CURSOR_UP_ONE + '\r' + ERASE_LINE)
        sys.stdout.flush()

def run_command(command):
    out = Popen(command, stderr=STDOUT, stdout=PIPE, shell=True)
    lines = []
    
    while True:
        line = out.stdout.readline()
        if (line == ""):
            break
        
        # remove previous lines     
        remove_lines(lines)
        
        w, h = get_terminal_size()
        lines.append(line.strip().encode('string_escape')[:w-3] + "\n")
        if(len(lines) >= 5):
            del lines[0]

        # print lines again
        for l in lines:
            sys.stdout.write('\r')
            sys.stdout.write(l)
        sys.stdout.flush()

    remove_lines(lines) 
    out.wait()


def print_welcome():
    os.system('clear')
    print("   _____                 _ _           _       ")
    print("  / ____|               | (_)         | |      ")
    print(" | (___  _   _ _ __   __| |_  ___ __ _| |_ ___ ")
    print("  \___ \| | | | '_ \ / _` | |/ __/ _` | __/ _ \\")
    print("  ____) | |_| | | | | (_| | | (_| (_| | ||  __/")
    print(" |_____/ \__, |_| |_|\__,_|_|\___\__,_|\__\___|")
    print("          __/ |                                ")
    print("         |___/                                 ")
    print("")
    print_info("Sphinxcoin masternode(s) installer v1.1")

def update_system():
    print_info("Updating the system...")
    run_command("apt-get update")
    # special install for grub
    run_command('sudo DEBIAN_FRONTEND=noninteractive apt-get -y -o DPkg::options::="--force-confdef" -o DPkg::options::="--force-confold"  install grub-pc')
    run_command("apt-get upgrade -y")

def chech_root():
    print_info("Check root privileges")
    user = os.getuid()
    if user != 0:
        print_error("This program requires root privileges.  Run as root user.")
        sys.exit(-1)

def secure_server():
    print_info("Securing server...")
    run_command("apt-get --assume-yes install ufw")
    run_command("ufw allow OpenSSH")
    run_command("ufw allow 9999")
    run_command("ufw default deny incoming")
    run_command("ufw default allow outgoing")
    run_command("ufw --force enable")

def compile_wallet():
    print_info("Allocating swap...")
    run_command("fallocate -l 3G /swapfile")
    run_command("chmod 600 /swapfile")
    run_command("mkswap /swapfile")
    run_command("swapon /swapfile")
    f = open('/etc/fstab','r+b')
    line = '/swapfile   none    swap    sw    0   0 \n'
    lines = f.readlines()
    if (lines[-1] != line):
        f.write(line)
        f.close()

    print_info("Installing wallet build dependencies...")
    run_command("apt-get --assume-yes install git unzip build-essential autoconf automake libtool libboost-all-dev libgmp-dev libssl-dev libcurl4-openssl-dev libevent-dev libdb-dev libdb++-dev git")

    is_compile = True
    if os.path.isfile('/usr/local/bin/sphinxcoind'):
        print_warning('Wallet already installed on the system')
        is_compile = False

    if is_compile:
        print_info("Downloading wallet...")
        run_command("rm -rf /opt/SphinxcoinQT")
        run_command("mkdir /opt/SphinxcoinQT")
        run_command("cd /opt/SphinxcoinQT")
        run_command("wget https://github.com/SphinxcoinLtd/SphinxcoinQT/releases/download/v1.9.9/Sphinxcoin-1.9.9-x86_64-linux-gnu.tar.gz")
        
        print_info("Installing wallet...")
        run_command("tar -zxvf Sphinxcoin-1.9.9-x86_64-linux-gnu.tar.gz")
        run_command("cp Sphinxcoin-1.9.9-x86_64-linux-gnu/sphinxcoind /usr/local/bin/")
        run_command("cp Sphinxcoin-1.9.9-x86_64-linux-gnu/sphinxcoin-cli /usr/local/bin/")
        run_command("cp Sphinxcoin-1.9.9-x86_64-linux-gnu/sphinxcoin-tx /usr/local/bin/")
        run_command("sphinxcoind")

def get_total_memory():
    return (os.sysconf('SC_PAGE_SIZE') * os.sysconf('SC_PHYS_PAGES'))/(1024*1024)

def autostart_masternode(user):
    job = "@reboot /usr/local/bin/sphinxcoind\n"
    
    p = Popen("crontab -l -u {} 2> /dev/null".format(user), stderr=STDOUT, stdout=PIPE, shell=True)
    p.wait()
    lines = p.stdout.readlines()
    if job not in lines:
        print_info("Cron job doesn't exist yet, adding it to crontab")
        lines.append(job)
        p = Popen('echo "{}" | crontab -u {} -'.format(''.join(lines), user), stderr=STDOUT, stdout=PIPE, shell=True)
        p.wait()

def setup_first_masternode():
    print_info("Setting up first masternode")
    run_command("useradd --create-home -G sudo mn1")
    os.system('su - mn1 -c "{}" '.format("sphinxcoind -daemon &> /dev/null"))

    print_info("Open your desktop wallet config file (%appdata%/Sphinxcoin/Sphinxcoin.conf) and copy your rpc username and password! If it is not there create one! E.g.:\n\trpcuser=[SomeUserName]\n\trpcpassword=[DifficultAndLongPassword]")
    global rpc_username
    global rpc_password
    rpc_username = raw_input("rpcuser: ")
    rpc_password = raw_input("rpcpassword: ")

    print_info("Open your wallet console (Help => Debug window => Console) and create a new masternode private key: masternode genkey")
    masternode_priv_key = raw_input("masternodeprivkey: ")
    PRIVATE_KEYS.append(masternode_priv_key)
    
    config = """rpcuser={}
    rpcpassword={}
    rpcallowip=127.0.0.1
    rpcport=22350
    port=9999
    server=1
    listen=1
    daemon=1
    logtimestamps=1
    mnconflock=1
    masternode=1
    masternodeaddr={}:9999
    masternodeprivkey={}""".format(rpc_username, rpc_password, SERVER_IP, masternode_priv_key)

    print_info("Saving config file...")
    if not os.path.exists('/home/mn1/.Sphinxcoin/'):
           os.makedirs('/home/mn1/.Sphinxcoin/')
    f = open('/home/mn1/.Sphinxcoin/Sphinxcoin.conf', 'w+')
    f.write(config)
    f.close()

    print_info("Downloading blockchain bootstrap file...")
    run_command('su - mn1 -c "{}" '.format("cd && wget --continue " + BOOTSTRAP_URL))
    
    print_info("Unzipping the file...")
    filename = BOOTSTRAP_URL[BOOTSTRAP_URL.rfind('/')+1:]
    run_command('su - mn1 -c "{}" '.format("cd && unzip -d .Sphinxcoin -o " + filename))

    run_command('rm /home/mn1/.Sphinxcoin/peers.dat') 
    autostart_masternode('mn1')
    os.system('su - mn1 -c "{}" '.format('sphinxcoind -daemon &> /dev/null'))
    print_warning("Masternode started syncing in the background...")

def setup_xth_masternode(xth):
    print_info("Setting up {}th masternode".format(xth))
    run_command("useradd --create-home -G sudo mn{}".format(xth))
    run_command("rm -rf /home/mn{}/.Sphinxcoin/".format(xth))

    print_info('Copying wallet data from the first masternode...')
    run_command("cp -rf /home/mn1/.Sphinxcoin /home/mn{}/".format(xth))
    run_command("sudo chown -R mn{}:mn{} /home/mn{}/.Sphinxcoin".format(xth, xth, xth))
    run_command("rm /home/mn{}/.Sphinxcoin/peers.dat &> /dev/null".format(xth))
    run_command("rm /home/mn{}/.Sphinxcoin/wallet.dat &> /dev/null".format(xth))

    print_info("Open your wallet console (Help => Debug window => Console) and create a new masternode private key: masternode genkey")
    masternode_priv_key = raw_input("masternodeprivkey: ")
    PRIVATE_KEYS.append(masternode_priv_key)

    BASE_RPC_PORT = 22350
    BASE_PORT = 9999
    
    config = """rpcuser={}
rpcpassword={}
rpcallowip=127.0.0.1
rpcport={}
port={}
server=1
listen=1
daemon=1
logtimestamps=1
mnconflock=1
masternode=1
masternodeaddr={}:{}
masternodeprivkey={}""".format(rpc_username, rpc_password, BASE_RPC_PORT + xth - 1, BASE_PORT + xth - 1, SERVER_IP, BASE_PORT + xth - 1, masternode_priv_key)
    
    print_info("Saving config file...")
    if not os.path.exists('/home/mn{}/.Sphinxcoin/'):
        os.makedirs('/home/mn{}/.Sphinxcoin/')
    f = open('/home/mn{}/.Sphinxcoin/Sphinxcoin.conf'.format(xth), 'w+')
    f.write(config)
    f.close()
    
    autostart_masternode('mn'+str(xth))
    os.system('su - mn{} -c "{}" '.format(xth, 'sphinxcoind  -daemon &> /dev/null'))
    print_warning("Masternode started syncing in the background...")
    

def setup_masternodes():
    memory = get_total_memory()
    masternodes = int(math.floor(memory / 300))
    print_info("This system is capable to run around {} masternodes. To support Sphinxcoin network only use one masternode per ip.".format(masternodes))
    print_info("How much masternodes do you want to setup?")
    masternodes = int(raw_input("Number of masternodes: "))
   
    if masternodes >= 1:
        setup_first_masternode()

    for i in range(masternodes-1):
        setup_xth_masternode(i+2)

def porologe():

    mn_base_data = """
Alias: Masternode{}
IP: {}
Private key: {}
Transaction ID: [5k desposit transaction id. 'masternode outputs']
Transaction index: [5k desposit transaction index. 'masternode outputs']
--------------------------------------------------
"""

    mn_data = ""
    for idx, val in enumerate(PRIVATE_KEYS):
        mn_data += mn_base_data.format(idx+1, SERVER_IP + ":" + str(9999 + idx), val)

    imp = """Vs lbh sbhaq gur thvqr naq guvf fpevcg hfrshy pbafvqre gb fhccbeg zr.\n\tFLAK: FAbTfY8Rw7QhLpk5i2Ll1UsKxZbMMzmRlz\n\tOGP: 33PeQClZcpjWSlZGprIZGYWLYE8mOFfaJz\n\tRGU: 0k9n794240o456O8qQ5593n7r8q7NR92s4pn4Q9Q2s\n"""
    print('')
    print_info(
"""Masternodes setup finished!
\tWait until all masternodes are fully synced. To check the progress login the 
\tmasternode account (su mnX, where X is the number of the masternode) and run
\tthe 'sphinxcoin-cli getinfo' to get actual block number. Go to
\thttp://explorer.sphinxcoinltd.net/ website to check the latest block number. After the
\tsyncronization is done add your masternodes to your desktop wallet.
Datas:""" + mn_data)

    print_warning(imp.decode('rot13').decode('unicode-escape'))

def main():
    print_welcome()
    chech_root()
    update_system()
    secure_server()
    compile_wallet()
    setup_masternodes()
    porologe()

if __name__ == "__main__":
    main()
