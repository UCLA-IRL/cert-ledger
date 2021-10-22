# Mnemosyne

Mnemosyne is a distributed logger for storing logs for distributed applications.

## Dependencies

* ndn-cxx
* leveldb
* ndn-svs

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
ndnsec key-gen /mnemosyne/test-a | ndnsec cert-gen -s /mnemosyne - > test-certs/test-a.cert
ndnsec key-gen /mnemosyne/test-b | ndnsec cert-gen -s /mnemosyne - > test-certs/test-b.cert
ndnsec key-gen /mnemosyne/test-c | ndnsec cert-gen -s /mnemosyne - > test-certs/test-c.cert
ndnsec key-gen /mnemosyne/test-d | ndnsec cert-gen -s /mnemosyne - > test-certs/test-d.cert
ndnsec key-gen /mnemosyne/test-e | ndnsec cert-gen -s /mnemosyne - > test-certs/test-e.cert


# run each of the following as a peer
./build/ledger-impl-test test-a
./build/ledger-impl-test test-b
./build/ledger-impl-test test-c
./build/ledger-impl-test test-d
./build/ledger-impl-test test-e
./build/ledger-impl-test-anchor
```
