# RandomLite (XRL) — Fast-Block Monero Privacy Network & Wallet

**RandomLite (XRL)** is a complete cryptocurrency network combining **RandomX (CPU Proof-of-Work)**, **Monero RingCT / Stealth Address Privacy**, **Fast 150-Second Block Times**, and a dedicated **CLI Wallet & Tokenomics Engine**.

---

## 📊 Tokenomics & Emission Curve

| Metric | Specification |
| :--- | :--- |
| **Base Money Supply** | **18,400,000 XRL** (18.4 Million XRL) |
| **Tail Emission Floor** | **0.6 XRL** per 150s block (~210,240 XRL / year perpetual) |
| **Emission Speed Factor** | `20` (Exponential decay adapted for 150s block target times) |
| **Target Block Time** | **150 seconds** (2.5 minutes like Litecoin) |
| **Spendable Output Lock** | **10 blocks** (25 minutes spendable age maturity) |
| **Atomic Units / Decimals** | `12` (`1 XRL = 1,000,000,000,000 atomic units`) |
| **Privacy Anonymity Set** | **Mandatory Ring Size 16** (1 real + 15 decoys via CLSAG) |
| **Range Proof Format** | **Bulletproofs+** (Zero-Knowledge confidential amounts) |

### Block Reward Emission Schedule
* **Height #0 (Genesis)**: `100.0 XRL`
* **Height #250,000 (~1.2 years)**: `11.30 XRL`
* **Height #500,000 (~2.4 years)**: `8.60 XRL`
* **Height #750,000 (~3.6 years)**: `6.55 XRL`
* **Height #1,000,000 (~4.8 years)**: `4.99 XRL`
* **Tail Emission Era**: `0.6 XRL` per block indefinitely to reward miners and preserve network security.

---

## 🛠️ Codebase & Executable Targets

```
c:\Users\danie\Desktop\randomx\build\
├── randomlited.exe             # Full P2P Node Daemon (Monero state machine + Zawy LWMA-3)
├── randomlite-miner.exe        # Standalone Multithreaded RandomX CPU Mining Client
└── randomlite-wallet-cli.exe   # Command Line Stealth Wallet & Tokenomics CLI
```

---

## ⚙️ Building & Execution

### Monero-based XRL fork

The production migration is under `vendor/monero` and uses Monero's canonical
crypto, transaction, wallet, LMDB, P2P, RPC, and RandomX code. Build that fork
with:

```cmd
.\build-monero.bat
```

This requires CMake and a MinGW/MSYS2 toolchain with Monero's Windows
dependencies. The original `build.bat` remains the legacy prototype build and
must not be used as a production network node.

### Compile all binaries
```cmd
.\build.bat
```

### 1. Launch Node Daemon
```cmd
.\build\randomlited.exe
```

### 2. Launch CPU Miner
```cmd
.\build\randomlite-miner.exe
```

### 3. Launch Wallet CLI
```cmd
.\build\randomlite-wallet-cli.exe
```

#### Wallet CLI Commands
* `address` — View primary `XRL4...` stealth address
* `balance` — View total & unlocked balance (10 block spendable maturity lock)
* `transfer <address> <amount>` — Construct & sign CLSAG 16 RingCT confidential transaction
* `seed` — Display 25-word recovery phrase
* `tokenomics` — Display supply curve & block reward metrics
* `history` — View transaction history log
