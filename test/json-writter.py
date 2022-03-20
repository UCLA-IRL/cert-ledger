#!/usr/bin/env python
import sys
import argparse
import os
import subprocess
import argparse
import json
from typing import Dict

ca_cert = ""

with open("test/cert-ledger-anchor.cert", "r", encoding='utf-8') as ca_encoded:
    ca_cert = ca_encoded.read()

with open("test/client.conf.example", "r+", encoding='utf-8') as client_conf:
    json_data = client_conf.read()
    json_config = json.loads(json_data)
    json_config['ca-list'][0]['certificate'] = ca_cert
    client_conf.seek(0)
    client_conf.write(json.dumps(json_config, indent=2))
    client_conf.truncate()