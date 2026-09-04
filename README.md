# RandomLite (XRL) — Fast-Block RandomX Privacy Chain

**RandomLite (XRL)** is a cryptocurrency built on the Monero v0.18.1.0 codebase
(`vendor/monero`) with its own genesis blocks, address format, network identity,
and emission schedule: **RandomX CPU mining**, **RingCT + stealth addresses**,
**150-second blocks**, and **mandatory ring size 16**.

**Status: working private testnet.** Distinct genesis (`bc8210c0…`), verified
2-node P2P sync with live block relay, mining, and wallets. Not yet a public
mainnet — see [docs/PRODUCTION_ROADMAP.md](docs/PRODUCTION_ROADMAP.md).

---

## Quickstart

### Build (Windows, LLVM-MinGW + vcpkg)

```cmd
.\build-monero.bat
```

Produces portable binaries in `build\monero-xrl\bin\` (all runtime DLLs staged
alongside — no PATH setup needed):

| Binary | Purpose |
|---|---|
| `monerod.exe` | Full node daemon |
| `monero-wallet-cli.exe` | Interactive wallet |
| `monero-wallet-rpc.exe` | Wallet RPC for services/exchanges |
| `xrl-genesis-tool.exe` | Deterministic genesis generator |

### Run the solo testnet (one click)

```cmd
.\xrl-miner.bat              :: daemon + 2-thread mining + wallet console
.\xrl-miner.bat --threads 8  :: more threads
.\xrl-miner.bat --no-wallet  :: daemon + mining only
```

### Host a seed node

```cmd
.\host-seed-node.bat         :: testnet seed, public P2P :48080, RPC localhost-only
```
(Run as Administrator to auto-create the firewall rule; on a VPS, open TCP 48080.)

### Join a hosted testnet

```cmd
.\join-testnet.bat <SEED_IP> [SEED2_IP] --threads 4
```

### Local 2-node test

```cmd
:: Node A (default ports 48080/48081/48082)
monerod.exe --testnet --allow-local-ip --data-dir testnet-data-v2

:: Node B (separate ports + data dir, peered to A)
monerod.exe --testnet --allow-local-ip --add-exclusive-node 127.0.0.1:48080 ^
  --p2p-bind-port 58080 --rpc-bind-port 58081 --zmq-rpc-bind-port 58082 ^
  --data-dir testnet-data-nodeB
```

---

## Chain parameters

| Parameter | Value |
|---|---|
| Consensus | RandomX PoW (CPU) |
| Block time | 150 s |
| Difficulty | LWMA, 720-block window |
| Base supply | 18,400,000 XRL |
| Tail emission | 0.6 XRL / block (perpetual) |
| Coinbase maturity | 10 blocks |
| Ring size | 16 (mandatory, HF v15) |
| Address prefixes | testnet `0x1b4` / `0x1b5` / `0x1b6` (addresses start with `X`) |
| Ports | mainnet 28080–28082 · testnet 48080–48082 · stagenet 38090–38092 |

Genesis blocks are deterministic and reproducible by anyone:

```cmd
build\monero-xrl\bin\xrl-genesis-tool.exe "RandomLite-XRL-Testnet-Genesis" 10001
```

---

## Repository layout

```
vendor/monero/        Production node/wallet source (Monero v0.18.1.0 + XRL diff)
build-monero.bat      AUTHORITATIVE production build
xrl-miner.bat         All-in-one solo testnet launcher
host-seed-node.bat    Public seed node host
join-testnet.bat      Testnet participant launcher
docs/PRODUCTION_ROADMAP.md   Rated next-steps plan (testnet → mainnet)
MONERO_XRL_MIGRATION.md      Migration status & consensus notes
src/                  LEGACY prototype — quarantined, not production
                      (disabled by default; cmake -DXRL_BUILD_PROTOTYPE=ON)
build.bat / build.sh  Legacy prototype build — do not use for real nodes
```

## The XRL diff vs upstream Monero

Small and auditable: genesis blocks + network IDs, address prefixes, emission
constants (150 s / 0.6 XRL tail), `MAP_JON_RPC_IF` macro + mining methods in the
`/json_rpc` dispatch map, emptied seed-IP list, `xrl-genesis-tool`, XRL branding.

## Security notes

- Never expose RPC (48081) or ZMQ (48082) publicly — P2P (48080) only.
- Script wallet password `passwordpass123` is testnet-only; change before real value.
- Do not reuse Monero mainnet keys/seed phrases on this chain.
