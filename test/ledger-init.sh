#!/bin/bash

# install customized ndncert
# mkdir dep && cd mkdir dep
# git clone https://github.com/tianyuan129/ndncert.git
# cd ndncert && git checkout v0.3
# ./waf configure
# sudo ./waf install && cd ..

help()
{
   echo "Usage: $0 -c ssl_cert -p ssl_prv -d name_component"
   echo -c "\t SSL Certificate"
   echo -p "\t Private key for the supplied SSL Certificate"
   echo -d "\t Name Component wish to have under /cert_ledger"
   exit 1 # Exit script after printing help
}

python3 test/json-writter.py
sudo cp test/client.conf.example /usr/local/etc/ndncert/client.conf

while getopts c:p:d: flag
do
    case "${flag}" in
        c) ssl_cert=${OPTARG};;
        p) ssl_prv=${OPTARG};;
        d) name_comp=${OPTARG};;
        ?) help;;
    esac
done

if [ -c "$ssl_cert" ] || [ -p "$ssl_prv" ] || [ -d "$name_comp" ]
then
   help
   exit 1
fi

echo "SSL Certificate path: $ssl_cert"
echo "Private key for SSL Certificate: $ssl_prv"
python3 test/auto.py -c $ssl_cert -p $ssl_prv -n "/cert_ledger/${name_comp}"
sleep 1

echo "\nNow begin the ledger part..."
sudo env NDN_LOG=INFO
./build/ledger-impl-test "/cert_ledger/${name_comp}"