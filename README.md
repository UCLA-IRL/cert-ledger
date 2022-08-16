# Cert Ledger

Cert Ledger is a distributed logger for storing Certificates/Revocation records for distributed trust anchors.

## Dependencies

* ndn-cxx
* ndn-svs
* NFD - to forward the NDN network

## Compile

```bash
./waf configure --with-tests
./waf
sudo ./waf install
```

## Run Tests

```bash
./waf configure --with-tests
./waf
sudo ./waf install
./build/unit-tests
```