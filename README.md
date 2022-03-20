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
ndnsec key-gen /cert-ledger | ndnsec cert-gen -s /cert-ledger - > cert-ledger-anchor.cert 

mkdir test-certs
ndnsec key-gen /cert-ledger/a | ndnsec cert-gen -s /cert-ledger - > test-certs/a.cert
ndnsec key-gen /cert-ledger/b | ndnsec cert-gen -s /cert-ledger - > test-certs/b.cert
ndnsec key-gen /hydra/test-logger | ndnsec cert-gen -s /cert-ledger - > test-certs/test-logger.cert

# need to serve certificate

# run each of the following as a peer
./build/app/cert-ledger-logger -l /cert-ledger/a
./build/app/cert-ledger-logger -l /cert-ledger/b
./build/test/cert-ledger-test-client -c /hydra/test-logger
```
