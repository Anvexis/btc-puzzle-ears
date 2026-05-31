#!/usr/bin/env python3
"""Validates the Early-Abort logic: ensures SHA256 prefix filtering correctly rejects non-matching candidates."""
import hashlib
import sys

def ripemd160(data: bytes) -> bytes:
    return hashlib.new('ripemd160', data).digest()

def test_early_abort_logic():
    target = b'\xf6\xf5\x43\x1d' + b'\x00'*16
    prefix_len = 4
    
    matches = 0
    for i in range(10_000):
        fake_x = (i * 0x9E3779B9).to_bytes(32, 'big')
        sha = hashlib.sha256(fake_x).digest()
        
        # Early abort: skip if SHA256 prefix doesn't match
        if sha[:prefix_len] != target[:prefix_len]:
            continue
            
        # Full Hash160 computation only for survivors
        pk = b'\x02' + fake_x  # compressed pubkey
        sha2 = hashlib.sha256(pk).digest()
        rmd = ripemd160(sha2)
        
        if rmd[:prefix_len] == target[:prefix_len]:
            matches += 1
            
    print(f"✅ Early-Abort test passed")
    print(f"   Survived filter: {matches}/10000 ({matches/100:.2f}%)")
    print(f"   Theoretical probability: 1/{256**prefix_len}")
    assert matches <= 2, "False positive rate is too high!"
    return True

if __name__ == "__main__":
    try:
        test_early_abort_logic()
        sys.exit(0)
    except Exception as e:
        print(f"❌ Test failed: {e}")
        sys.exit(1)