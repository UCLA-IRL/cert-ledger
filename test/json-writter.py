#!/usr/bin/env python
import sys
import argparse
import os
import subprocess
import argparse
import json
from typing import Dict

ca_cert = ""

with open("mnemosyne-anchor.cert", "r", encoding='utf-8') as ca_encoded:
    ca_cert = ca_encoded.read()

with open("client.conf.example", "r+", encoding='utf-8') as client_conf:
    json_data = client_conf.read()
    json_config = json.loads(json_data)
    print(json_config['ca-list'][0]['certificate'])

    json_config['ca-list'][0]['certificate'] = ca_cert
    client_conf.seek(0)
    client_conf.write(json.dumps(json_config, indent=2))
    client_conf.truncate()