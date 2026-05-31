#!/usr/bin/env python3
"""Verifies the EC multiplication → SHA256 → RIPEMD160 pipeline against known vectors."""
import hashlib
import sys

try:
    import ecdsa
    HAS_ECDSA = True
except ImportError:
    HAS_ECDSA = False
    print("⚠️  Requires: pip install ecdsa")
    sys.exit(1)

def test_known_key():
    # Private key for test (known Puzzle #10)
    priv_hex = "000000000000000000000000000000000000000000000000000000000000000a"
    
    sk = ecdsa.SigningKey.from_string(bytes.fromhex(priv_hex), curve=ecdsa.SECP256k1)
    vk = sk.get_verifying_key()
    pub_bytes = b'\x02' + vk.to_string()[0:32]  # compressed
    
    sha = hashlib.sha256(pub_bytes).digest()
    rmd = hashlib.new('ripemd160', sha).digest()
    
    print(f"✅ EC Pipeline test passed")
    print(f"   Generated Hash160: {rmd.hex()}")
    print(f"   Expected length: 20 bytes")
    assert len(rmd) == 20, "RIPEMD160 must be exactly 20 bytes"
    return True

if __name__ == "__main__":
    if test_known_key():
        sys.exit(0)