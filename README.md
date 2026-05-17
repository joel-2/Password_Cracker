# Password Cracker in C

A multithreaded password cracker written in C, built from scratch as a cybersecurity learning project. Supports offline hash cracking with multiple algorithms and parallel execution via pthreads.

---

## Current Status

| Phase | Feature | Status |
|-------|---------|--------|
| 1 | MD5 dictionary attack | ✅ Complete |
| 2 | Multi-algorithm support (MD5, SHA1, SHA256, SHA512) | ✅ Complete |
| 3 | Multithreading with pthreads | ✅ Complete |
| 4 | Rules engine (leet speak, capitalisation, suffixes) | 🔄 In Progress |
| 5 | Brute force mode | ⬜ Planned |
| 6 | Online cracker (SSH, FTP, HTTP) | ⬜ Planned |
| 7 | CLI polish, JSON output, resume support | ⬜ Planned |

---

## Features

- **Dictionary attack** — reads a wordlist line by line, never loads the full file into memory (lazy reading via `fgets`)
- **Multi-algorithm** — supports MD5, SHA1, SHA256, and SHA512 via OpenSSL's EVP API
- **Multithreaded** — parallel cracking using POSIX threads (pthreads); thread count is configurable at runtime
- **Early exit** — shared stop flag ensures all threads halt immediately when a match is found, minimising wasted computation
- **Thread-safe** — mutex-protected file reading and result output prevent race conditions

---

## Why C?

This project is intentionally written in C rather than Python for several reasons:

- Raw hashing speed — no interpreter overhead, direct CPU usage
- Real systems programming experience — manual memory management, pointers, pthreads
- Mirrors how production tools like Hashcat and THC-Hydra are actually built
- Forces a deep understanding of what higher-level languages abstract away

---

## Dependencies

| Library | Purpose | Install |
|---------|---------|---------|
| gcc | C compiler | `sudo apt install gcc -y` |
| gdb | Debugger | `sudo apt install gdb -y` |
| libssl-dev | OpenSSL EVP API for hashing | `sudo apt install libssl-dev -y` |
| libpthread | POSIX threads (pthreads) | Included with glibc on Linux |

Install all at once:
```bash
sudo apt update
sudo apt install gcc gdb libssl-dev -y
```

---

## Building

```bash
gcc src/hash.c -o hash_cracker -lssl -lcrypto -lpthread
```

---

## Usage

```bash
./hash_cracker <wordlist> <hash> <algorithm> <threads>
```

### Arguments

| Argument | Description | Example |
|----------|-------------|---------|
| `wordlist` | Path to wordlist file | `rockyou.txt` |
| `hash` | Target hash to crack | `5f4dcc3b...` |
| `algorithm` | Hash algorithm | `md5`, `sha1`, `sha256`, `sha512` |
| `threads` | Number of threads to use | `4` |

### Examples

**Crack an MD5 hash:**
```bash
./hash_cracker rockyou.txt 5f4dcc3b5aa765d61d8327deb882cf99 md5 4
```

**Crack a SHA256 hash:**
```bash
./hash_cracker rockyou.txt 5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8 sha256 4
```

**Crack a SHA512 hash:**
```bash
./hash_cracker rockyou.txt 569554a74cfa627e97567fd227897b81a56449ffed56c345c26b9fff67ea65e8b2984d21a65d911b16b25c54809201b6dda981e98ce96fb1849e318f0c0f47a5 sha512 8
```

### Generating test hashes

```bash
echo -n "password" | md5sum
echo -n "password" | sha256sum
echo -n "password" | sha512sum
```

---

## Wordlists

This project does not include wordlists. Download rockyou.txt separately:

```bash
curl -L -o rockyou.txt https://github.com/brannondorsey/naive-hashcat/releases/download/data/rockyou.txt
```

rockyou.txt is excluded from this repository via `.gitignore` due to its 133MB file size.

---

## How It Works

### Architecture

```
hash.c
├── bytes_to_hex()     - converts raw hash bytes to hex string for comparison
├── crack()            - hashes a single word using OpenSSL EVP API and compares to target
├── CrackJob struct    - shared state passed to all threads (wordlist, target hash, algorithm, mutex, stop flag)
├── worker()           - thread function; reads lines, hashes, checks stop flag
└── main()             - parses args, initialises job, spawns threads, waits for completion
```

### OpenSSL EVP API

Rather than using deprecated single-function calls like `MD5()` or `SHA256()`, this project uses OpenSSL's EVP (Envelope) API — a unified interface for all hash algorithms:

```c
EVP_MD_CTX *ctx = EVP_MD_CTX_new();
EVP_DigestInit(ctx, EVP_sha256());
EVP_DigestUpdate(ctx, word, strlen(word));
EVP_DigestFinal(ctx, digest, &digest_len);
EVP_MD_CTX_free(ctx);
```

Switching algorithms is a single function swap (`EVP_md5()`, `EVP_sha1()`, etc.) — the rest of the code is identical.

### Multithreading

Threads share a single `CrackJob` struct containing:
- A shared file handle to the wordlist
- A `pthread_mutex_t` protecting file reads and result output
- A `volatile int found` stop flag

Each thread locks the mutex, reads the next line, increments the attempt counter, then releases the lock before hashing. When any thread finds a match it sets `found = 1` inside the lock — all other threads check this flag at the top of their loop and exit immediately.

**Note:** The attempts counter is approximate due to parallel execution — threads may read ahead slightly before the stop flag propagates. This is expected behaviour.

### Threading performance

Tested on a 22-core machine cracking a SHA512 hash against rockyou.txt (14M passwords):

| Threads | Time |
|---------|------|
| 1 | ~4m+ |
| 8 | 1m05s |
| 22 | 1m25s |

Performance peaks around 8 threads due to mutex contention on the shared file handle — threads queue up waiting to read the next line. Beyond that point, context switching overhead outweighs the parallelism benefit. A future optimisation (work partitioning) will pre-split the wordlist so each thread reads independently with no mutex contention.

---

## Planned Features

- **Rules engine** — mutate wordlist entries with leet speak, capitalisation, suffix appending, reversal
- **Brute force mode** — generate all combinations of a charset up to a given length
- **Online cracker** — SSH, FTP, and HTTP brute forcing via libssh and raw sockets
- **Work partitioning** — pre-split wordlist into per-thread chunks to eliminate mutex contention
- **Resume support** — checkpoint progress to disk and continue interrupted scans
- **JSON output** — structured results file consistent with companion recon tool

---

## Legal Disclaimer

This tool is for **educational purposes only**. Only use it against systems you own or have explicit written permission to test. Unauthorised use against systems you do not own is illegal under the Computer Misuse Act and equivalent legislation in your jurisdiction.

---

## Acknowledgements

- [OpenSSL](https://www.openssl.org/) — EVP hashing API
- [rockyou.txt](https://github.com/brannondorsey/naive-hashcat) — wordlist used for testing
- Built as part of a cybersecurity project at South East Technological University