// src/puzzle71_solver.cpp
// Bitcoin Puzzle #71 Solver v3.1 | libsecp256k1 + Early-Abort
// Compile: make
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <secp256k1.h>

using u8 = uint8_t;
using u64 = uint64_t;

constexpr u8 TARGET_HASH160[20] = {
    0xf6,0xf5,0x43,0x1d,0x25,0xbb,0xf7,0xb1,0x2e,0x8a,
    0xdd,0x9a,0xf5,0xe3,0x47,0x5c,0x44,0xa0,0xa5,0xb8
};

struct Stats {
    std::atomic<u64> keys_tested{0};
    std::atomic<u64> aborts{0};
    std::atomic<u64> full_checks{0};
    std::atomic<bool> found{false};
    std::mutex mtx;
    u8 found_priv[32] = {0};
};

// OpenSSL 3.0+ compatible RIPEMD160
void ripemd160_evp(const u8* in, size_t len, u8* out) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_ripemd160(), nullptr);
    EVP_DigestUpdate(ctx, in, len);
    unsigned int out_len = 20;
    EVP_DigestFinal_ex(ctx, out, &out_len);
    EVP_MD_CTX_free(ctx);
}

void worker(int id, Stats& stats, u64 start, u64 end) {
    // Thread-local context (fast & thread-safe)
    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
    if (!ctx) { std::cerr << "[Thread " << id << "] Failed to init secp256k1\n"; return; }

    u8 priv[32] = {0};
    u8 pub_comp[33];
    size_t pub_len = 33;
    secp256k1_pubkey pubkey_obj;
    u8 sha[32], rmd[20];

    for (u64 k = start; k < end && !stats.found.load(); ++k) {
        // 64-bit counter → 32-byte big-endian private key
        memset(priv, 0, 32);
        for (int i = 0; i < 8; ++i) priv[31 - i] = (k >> (8 * i)) & 0xFF;

        // 1. Create pubkey object
        if (secp256k1_ec_pubkey_create(ctx, &pubkey_obj, priv) == 0) continue;
        
        // 2. Serialize to compressed format (33 bytes)
        if (secp256k1_ec_pubkey_serialize(ctx, pub_comp, &pub_len, &pubkey_obj, SECP256K1_EC_COMPRESSED) == 0) continue;

        stats.keys_tested.fetch_add(1, std::memory_order_relaxed);

        // SHA256(compressed_pubkey)
        SHA256(pub_comp, pub_len, sha);
        
        // RIPEMD160(SHA256(pubkey))
        ripemd160_evp(sha, 32, rmd);

        // Early Abort: skip 99.6% of candidates here
        if (rmd[0] != TARGET_HASH160[0]) {
            stats.aborts.fetch_add(1, std::memory_order_relaxed);
            continue;
        }

        stats.full_checks.fetch_add(1, std::memory_order_relaxed);
        if (memcmp(rmd, TARGET_HASH160, 20) == 0) {
            std::lock_guard<std::mutex> lock(stats.mtx);
            if (!stats.found.load()) {
                stats.found = true;
                memcpy(stats.found_priv, priv, 32);
                
                std::ofstream out("FOUNDKEY.TXT");
                out << "🔑 Bitcoin Puzzle #71 SOLVED!\n";
                out << "Private Key (hex): ";
                for(int i=0; i<32; ++i) out << std::hex << std::setfill('0') << std::setw(2) << (int)priv[i];
                out << "\nAddress: 1PWo3JeB9jrGwfHDNpdGK54CRas7fsVzXU\n";
                out.close();
                std::cout << "\n🎉 KEY FOUND! Saved to FOUNDKEY.TXT\n";
            }
        }
    }
    secp256k1_context_destroy(ctx);
}

int main() {
    std::cout << "🚀 EARS Solver v3.1 | libsecp256k1 + Early-Abort\n";
    std::cout << "📐 Range: [2^70, 2^71) | Target: 1PWo3JeB9jrGwfHDNpdGK54CRas7fsVzXU\n\n";

    Stats stats;
    std::vector<std::thread> workers;
    auto t0 = std::chrono::high_resolution_clock::now();
    auto last = t0;

    const u64 RANGE_START = 0x4000000000000000ULL; // 2^70
    const u64 RANGE_END   = 0x8000000000000000ULL; // 2^71
    const int THREADS = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 8;
    const u64 CHUNK = (RANGE_END - RANGE_START) / THREADS;

    for (int i = 0; i < THREADS; ++i) {
        u64 start = RANGE_START + i * CHUNK;
        u64 end   = (i == THREADS - 1) ? RANGE_END : start + CHUNK;
        workers.emplace_back(worker, i, std::ref(stats), start, end);
    }

    while (!stats.found.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto now = std::chrono::high_resolution_clock::now();
        double el = std::chrono::duration<double>(now - last).count();
        if (el < 1.0) continue;

        u64 tested = stats.keys_tested.exchange(0, std::memory_order_relaxed);
        u64 aborts = stats.aborts.exchange(0, std::memory_order_relaxed);
        u64 full   = stats.full_checks.exchange(0, std::memory_order_relaxed);
        double kps = tested / el;
        double abort_pct = tested ? (100.0 * aborts / tested) : 0.0;

        std::cout << "\r⚡ " << std::fixed << std::setprecision(1) << kps/1000.0 << "k keys/s | "
                  << "Abort: " << std::setprecision(1) << abort_pct << "% | "
                  << "Full: " << full << " | "
                  << "Elapsed: " << std::setprecision(0) << std::chrono::duration<double>(now-t0).count()/3600.0 << "h" << std::flush;
        last = now;
    }

    for (auto& t : workers) if (t.joinable()) t.join();
    std::cout << "\n✅ Search finished.\n";
    return stats.found.load() ? 0 : 1;
}