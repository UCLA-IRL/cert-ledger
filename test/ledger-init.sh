# install customized ndncert
# mkdir dep && cd mkdir dep
# git clone https://github.com/tianyuan129/ndncert.git
# cd ndncert && git checkout v0.3
# ./waf configure
# sudo ./waf install && cd ..
sudo cp client.conf.example /usr/local/etc/ndncert/client.conf

echo "Input your SSL Certificate:"
read ssl_cert
echo "Your SSL Certificate path is - $ssl_cert\n"

echo "Input your corresponding private key for SSL Certificate:"
read ssl_prv
echo "Your private key path is - $ssl_prv\n"

echo "Input your name component after /mnemosyne:"
read comp
echo "Your private key path is - $comp\n"

python3 auto.py -c $ssl_cert -p $ssl_prv -n "/mnemosyne/$comp"
ndnsec-cert-dump -i "/mnemosyne/$comp"- > test-certs/$comp.cert
./build/ledger-impl-test $comp