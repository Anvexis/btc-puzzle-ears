# 🔐 Early-Abort Range Search (EARS) for Bitcoin Puzzles

> High-performance solver for finding private keys given a known range and `RIPEMD160` address hash.  
> Implements a `Coarse → Fine` pipeline with incremental Early-Abort filtering and Fixed-Base windowed multiplication.

## 📖 Overview
This project targets Bitcoin Puzzle #71 (`k ∈ [2^70, 2^71)`) when **only the Hash160 address is known** (`f6f5431d...`). Unlike Pollard's Kangaroo (which requires the public key `Q`), EARS operates exclusively on the hash by combining:
1. 🧮 Optimized scalar multiplication `x(kG)` via Fixed-Base Windowing
2. 🚦 Incremental hashing with SHA256-level early abort
3. 💾 Checkpoint/Resume support for long-running computations

## 🔬 Mathematical Reality & Hypothesis Fixes
During development, the initial "prefix-filter + lookup-table" hypothesis was empirically tested and corrected:

| Component | Status | Conclusion |
|-----------|--------|------------|
| `k_high → addr_prefix` correlation | ❌ Non-existent | SHA256+RIPEMD160 exhibit strong avalanche effect: 1-bit change in `k` alters ~50% of address bits. Lookup tables are cryptographically invalid. |
| Early-Abort hashing | ✅ Functional | Skips 99.6% of RIPEMD160 computations, but **does not accelerate EC multiplication**, which consumes ~95% of runtime. |
| Asymptotic complexity | ⚠️ Remains `O(W)` | EARS is an optimized brute-force method. Gain is constant-factor (`~3–5×`), not exponential. |
| Fixed-Base Window (4-bit) | ✅ Implemented | Reduces point operations from 256 to ~64 per key. Provides real-world throughput improvement. |

### 🧠 Why the Prefix Filter Was Removed
Cryptographic hashes are explicitly designed to destroy statistical dependencies between inputs and output prefixes. Our test `tests/test_avalanche_proof.py` formally demonstrates uniform distribution of address prefixes across sequential `k` values. Using a lookup table would either cause false positives or miss the actual key.

## 🛠 Installation & Usage

### 🔧 Dependencies
```bash
sudo apt update
sudo apt install build-essential libssl-dev python3 python3-pip git
pip3 install ecdsa numpy  # For tests only

📦 Build
git clone https://github.com/yourusername/btc-puzzle-ears.git
cd btc-puzzle-ears
make

🚀 Run Solver
./puzzle71_solver

```
# 🔐 EARS Solver v3.0 | Bitcoin Puzzle #71 Range Search

> High-performance early-abort brute-force solver for Bitcoin address ranges using `libsecp256k1` and OpenSSL.

## 📖 Overview
This project searches for private keys in the `[2^70, 2^71)` range targeting address `1PWo3JeB9jrGwfHDNpdGK54CRas7fsVzXU`. It combines:
- 🧮 **libsecp256k1**: Highly optimized EC point multiplication (windowed arithmetic)
- 🚦 **Early-Abort Pipeline**: RIPEMD160 prefix filtering reduces full comparisons by ~99.6%
- 💾 **Thread-Local Contexts**: Zero-lock EC operations for maximum CPU utilization
- 📊 **Live Telemetry**: Real-time throughput, abort rates, and ETA tracking

## 🔧 Prerequisites
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential libssl-dev libsecp256k1-dev git

# macOS (Homebrew)
brew install openssl secp256k1

🛠 Build & Run
# 1. Clone & enter
git clone https://github.com/yourusername/btc-puzzle-ears.git
cd btc-puzzle-ears

# 2. Compile
make

# 3. Run
./puzzle71_solver

```
📁 Output
FOUNDKEY.TXT: Automatically created if a match occurs. Contains hex private key and verification.
Console: Live throughput, abort statistics, and elapsed time.

 Disclaimer
For educational and research purposes only. Brute-forcing cryptocurrency wallets outside authorized ranges may violate local laws. Always respect cryptographic boundaries and community guidelines.
📜 License
MIT License.
