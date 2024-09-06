import base64
import requests
import json
import sys

revision = 20200327
file     = "CC2538_ZNP_{rev}.bin".format(rev=revision)
uri      = "http://localhost/utility/0/config"

def fw_load(file):
    try:
        with open(file, 'r') as reader:
            return base64.b64encode(bytes(reader.read()))
    except IOError as e:
        print("Failed to open file (%s)." % e)

def fw_format(data, revision):
    # PluginRPCTimeout (ms) = 6 * 60 * 1000 = 6mins
    data = {
        'header': {'content-type': 'application/json', 'Accept-Charset': 'UTF-8', 'Timeout' : (10*60), 'PluginRPCTimeout': (6*60*1000)},
        'payload': {'method': 'iHomePluginIf_IOCTL', 'id': 'zigbee-1', 'params': [ 3, {'file': data, 'revision': revision}]}
    }
    return data if data else None

def fw_upgrade(file, revision, uri):
    packet = fw_format(fw_load(file), revision)
    resp = requests.post(uri, data=json.dumps(packet['payload']), headers=packet['header'], timeout=(10*60))
    if resp.status_code != 200:
        print("Zbee: Failed to send upgrade request to Framework! (%s)" % resp.status_code)
    return True if resp.status_code == 200 else False

if __name__ == "__main__":
    resp = fw_upgrade(file, revision, uri)
    if (resp == False):
        sys.exit(1)



# SBL: CC2538_SBL.bin
# ZNP: CC2538_ZNP_03272020.bin
# Combine: SBL + ZNP = CC2538_SBL_ZNP_03272020.bin
# copy /b CC2538_ZNP_20200327.bin + CC2538_SBL.bin CC2538_SBL_ZNP_20200327.bin
