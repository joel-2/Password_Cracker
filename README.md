# Password Cracker in C

A multithreaded password cracker written in C, built from scratch as a cybersecurity learning project. Supports offline hash cracking with multiple algorithms, parallel execution via pthreads, and a rules engine for password mutation.

---

## Current Status

| Phase | Feature | Status |
|-------|---------|--------|
| 1 | MD5 dictionary attack | ✅ Complete |
| 2 | Multi-algorithm support (MD5, SHA1, SHA256, SHA512) | ✅ Complete |
| 3 | Multithreading with pthreads | ✅ Complete |
| 4 | Rules engine (leet speak, capitalisation, suffixes, combined) | ✅ Complete |
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
- **Rules engine** — mutates each wordlist entry into variants before hashing; 10 built-in rules covering the most common password patterns

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
| libpthread | POSIX threads (included with glibc) | Pre-installed on Linux |

Install all at once:
```bash
sudo apt update
sudo apt install gcc gdb libssl-dev -y
```

---

## Project Structure

```
Password_Cracker/
├── src/
│   ├── hash.c          # main cracking engine — threading, EVP hashing, CLI
│   └── rules.c         # mutation rules engine
├── include/
│   └── rules.h         # rule function declarations and RuleFn typedef
├── test.txt            # small wordlist for testing
├── .gitignore          # excludes rockyou.txt and binaries
└── README.md
```

---

## Building

```bash
gcc src/hash.c src/rules.c -I include -o hash_cracker -lssl -lcrypto -lpthread
```

---

## Usage

```bash
./hash_cracker <wordlist> <hash> <algorithm> <threads> [--rules]
```

### Arguments

| Argument | Description | Example |
|----------|-------------|---------|
| `wordlist` | Path to wordlist file | `rockyou.txt` |
| `hash` | Target hash to crack | `5f4dcc3b...` |
| `algorithm` | Hash algorithm | `md5`, `sha1`, `sha256`, `sha512` |
| `threads` | Number of threads to use | `4` |
| `--rules` | Enable rules engine (optional) | `--rules` |

### Examples

**Basic dictionary attack — MD5:**
```bash
./hash_cracker rockyou.txt 5f4dcc3b5aa765d61d8327deb882cf99 md5 4
```

**Dictionary attack with rules — SHA256:**
```bash
./hash_cracker rockyou.txt 008c70392e3abfbd0fa47bbc2ed96aa99bd49e159727fcba0f2e6abeb3a9d601 sha256 4 --rules
```

**SHA512 with maximum threads:**
```bash
./hash_cracker rockyou.txt <hash> sha512 8
```

### Generating test hashes

```bash
echo -n "password"    | md5sum
echo -n "password"    | sha256sum
echo -n "Password123" | sha256sum   # tests rule_cap_append_123
echo -n "P@$$w0rd123" | sha256sum   # tests rule_combined
```

---

## Rules Engine

When `--rules` is passed, each word from the wordlist is mutated into variants before hashing. This lets the cracker find passwords that aren't in the wordlist but are derived from common words.

### Built-in Rules

| Rule | Input | Output | Description |
|------|-------|--------|-------------|
| `rule_capitalise` | `password` | `Password` | Capitalises first letter |
| `rule_uppercase` | `password` | `PASSWORD` | All uppercase |
| `rule_leet` | `password` | `p@$$w0rd` | Leet speak substitutions |
| `rule_append_123` | `password` | `password123` | Appends 123 |
| `rule_append_exclamation` | `password` | `password!` | Appends ! |
| `rule_append_year` | `password` | `password2024` | Appends current year |
| `rule_reverse` | `password` | `drowssap` | Reverses the word |
| `rule_combined` | `password` | `P@$$w0rd123` | Capitalise + leet + append 123 |
| `rule_cap_append_123` | `password` | `Password123` | Capitalise + append 123 |
| `rule_caps_numbers_append` | `password` | `PASSWORD123` | Uppercase + append 123 |

### Leet Speak Substitutions

| Character | Replacement |
|-----------|-------------|
| a / A | @ |
| e / E | 3 |
| i / I | 1 |
| o / O | 0 |
| s / S | $ |
| t / T | 7 |

### Adding Custom Rules

Rules are function pointers with a consistent signature — adding a new rule takes two steps:

**1. Write the function in `src/rules.c`:**
```c
void rule_my_custom(const char *word, char *out) {
    sprintf(out, "%s@123", word);   // e.g. appends @123
}
```

**2. Register it in the rules array:**
```c
RuleFn rules[] = {
    rule_capitalise,
    // ... existing rules ...
    rule_my_custom,    // just add it here
};
```

`rule_count` updates automatically via `sizeof(rules) / sizeof(rules[0])` — no other changes needed.

---

## How It Works

### Architecture

```
hash.c
├── bytes_to_hex()     — converts raw hash bytes to hex string for comparison
├── crack()            — hashes a single word using OpenSSL EVP API and compares to target
├── CrackJob struct    — shared state passed to all threads
├── worker()           — thread function; reads lines, applies rules, checks stop flag
└── main()             — parses args, initialises job, spawns threads, waits for completion

rules.c
├── rule_*()           — individual mutation functions
└── rules[]            — function pointer array; engine iterates this automatically
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

Switching algorithms is a single function swap — the rest of the code is identical.

### Multithreading

Threads share a single `CrackJob` struct containing a shared file handle, a `pthread_mutex_t`, and a `volatile int found` stop flag. Each thread locks the mutex to read the next line, then releases it before hashing. When any thread finds a match it sets `found = 1` — all other threads check this flag at the top of their loop and exit immediately.

### Threading Performance

Tested on a 22-core machine cracking a SHA512 hash against rockyou.txt (14M passwords):

| Threads | Time |
|---------|------|
| 1 | ~4m+ |
| 8 | 1m05s |
| 22 | 1m25s |

Performance peaks around 8 threads on this machine due to mutex contention on the shared file handle. Beyond that, context switching overhead outweighs the parallelism benefit.

---

## Wordlists

This project does not include wordlists. Download rockyou.txt separately:

```bash
curl -L -o rockyou.txt https://github.com/brannondorsey/naive-hashcat/releases/download/data/rockyou.txt
```

rockyou.txt is excluded from this repository via `.gitignore` due to its 133MB file size.

---

## Planned Features

- **Brute force mode** — generate all combinations of a charset up to a given length
- **Online cracker** — SSH, FTP, and HTTP brute forcing via libssh and raw sockets
- **Work partitioning** — pre-split wordlist into per-thread chunks to eliminate mutex contention
- **Resume support** — checkpoint progress to disk and continue interrupted scans
- **JSON output** — structured results file

---

## Legal Disclaimer

This tool is for **educational purposes only**. Only use it against systems you own or have explicit written permission to test. Unauthorised use against systems you do not own is illegal under the Computer Misuse Act and equivalent legislation in your jurisdiction.

---

## Acknowledgements

- [OpenSSL](https://www.openssl.org/) — EVP hashing API
- [rockyou.txt](https://github.com/brannondorsey/naive-hashcat) — wordlist used for testing
- Built as part of a fun cybersecurity project.