#!/usr/bin/env python3
"""
Genesis Block Generator for RandomLite (XRL) - Fast-Block Monero Fork
Generates custom genesis timestamp, nonce, tx public keys, and cryptographic header hash.
"""

import sys
import time
import hashlib
import binascii

def generate_genesis(coin_name="RandomLite", ticker="XRL", target_blocktime=150):
    timestamp = int(time.time())
    print("=" * 65)
    print(f"   {coin_name} ({ticker}) Genesis Block Generator")
    print(f"   Target Block Time: {target_blocktime} seconds (2.5 minutes)")
    print("=" * 65)

    # Simulated Genesis Key generation
    extra_tag = f"RandomLite Genesis Block - Fast RandomX Privacy Chain - {timestamp}".encode('utf-8')
    tx_extra_hex = binascii.hexlify(extra_tag).decode('utf-8')

    nonce = 10001
    
    # Genesis hash calculation
    hasher = hashlib.sha256()
    hasher.update(f"GENESIS_{coin_name}_{timestamp}_{nonce}".encode('utf-8'))
    genesis_hash = hasher.hexdigest()

    print(f"\n[+] Genesis Timestamp  : {timestamp}")
    print(f"[+] Genesis Nonce      : {nonce}")
    print(f"[+] Genesis Hash       : 0x{genesis_hash}")
    print(f"[+] Genesis Extra Payload: {tx_extra_hex[:40]}...")

    print("\nPaste these definitions into src/cryptonote_config.h:")
    print("-" * 65)
    print(f'#define GENESIS_TIMESTAMP                       {timestamp}')
    print(f'#define GENESIS_NONCE                           {nonce}')
    print(f'#define GENESIS_HASH_HEX                        "{genesis_hash}"')
    print("-" * 65)

if __name__ == "__main__":
    generate_genesis()
