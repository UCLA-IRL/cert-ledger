# Cert Ledger

Cert Ledger is a distributed logger for storing Certificates/Revocation records for distributed trust anchors.

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
ndnsec key-gen /cert-ledger | ndnsec cert-gen -s /cert-ledger - > cert-ledger-anchor.cert 

mkdir test-certs
ndnsec key-gen /cert-ledger/a | ndnsec cert-gen -s /cert-ledger - > test-certs/a.cert
ndnsec key-gen /cert-ledger/b | ndnsec cert-gen -s /cert-ledger - > test-certs/b.cert

# need to serve certificate

# run each of the following as a peer
./build/test/cert-ledger-test -l /cert-ledger/a
./build/test/cert-ledger-test -l /cert-ledger/b
```
