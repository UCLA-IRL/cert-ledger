# Mnemosyne

Mnemosyne is a distributed logger for storing logs for distributed applications.

## Dependencies

* ndn-cxx
* leveldb
* ndn-svs (develop branch)

* NFD - to forward the NDN network

## Compile

```bash
mkdir build && cd build
cmake ..
make
```

To run the test files

```bash

# configure NFD
nfd-start

# generate keys and certificates
ndnsec key-gen /mnemosyne | ndnsec cert-gen -s /mnemosyne - > mnemosyne-anchor.cert 

mkdir test-certs
ndnsec key-gen /mnemosyne/a | ndnsec cert-gen -s /mnemosyne - > test-certs/a.cert
ndnsec key-gen /mnemosyne/b | ndnsec cert-gen -s /mnemosyne - > test-certs/b.cert


# run each of the following as a peer
./build/ledger-impl-test /mnemosyne/a
./build/ledger-impl-test /mnemosyne/b
```
