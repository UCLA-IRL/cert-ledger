
#!/bin/bash

# generate keys and certificates
ndnsec delete /cert_ledger
ndnsec key-gen /cert_ledger | ndnsec cert-gen -s /cert_ledger - > ../cert_ledger-anchor.cert

# deploy customized ndncert ca
# mkdir dep && cd mkdir dep
# git clone https://github.com/tianyuan129/ndncert.git
# cd ndncert && git checkout v0.3
# ./waf configure
# sudo ./waf install && cd ..
cp ca.conf.example /usr/local/etc/ndncert/ca.conf

export NDN_LOG="ndncert.*=TRACE"
ndncert-ca-server
