# RandomLite (XRL) — Testnet → Production Roadmap

> **Audience:** future agents/developers continuing this codebase.
> **State as of 2026-09-04:** private testnet fully working — distinct genesis
> (`bc8210c0…`), XRL address prefixes (`0x1b4` testnet), verified 2-node P2P sync
> with live block relay, deterministic `xrl-genesis-tool`, DLL-staged portable
> binaries. Authoritative build: `build-monero.bat` (vendored Monero v0.18.1.0).
> The legacy `src/` prototype is quarantined (`XRL_BUILD_PROTOTYPE=OFF`) — **never
> ship its binaries**.

Each section is rated **X/10** (10 = production-grade) with the gap analysis.

---

## 1. Consensus & Chain Identity — 8/10

**Done:** distinct deterministic genesis per network; XRL ports/IDs; emission
(150 s target, 720-block LWMA, 0.6 XRL tail); ring size 16 enforced at HF v15;
`/json_rpc` mining dispatch fixed (`MAP_JON_RPC_IF`).

**Needs improving:**
- **RandomX seed epoch is 2048, not the spec'd 1440.** `rx_seedheight()` in
  `vendor/monero/src/cryptonote_core/cryptonote_tx_utils.cpp` requires a power of
  two. Changing to 1440 is a hard consensus change: rewrite the seed-height
  schedule, regenerate all test vectors, and audit `rx_seedheight` callers
  (block verification + mining paths). Do this *before* any public chain history
  exists — it cannot be changed after launch without a hard fork.
- **No consensus regression vectors.** Add unit tests asserting genesis hash,
  first-block reward (17.547607421875 XRL), and difficulty retarget edges.
- **Hardfork schedule is Monero's** (`HF_VERSION_*` table in
  `cryptonote_config.h`). XRL launches at v1 — decide whether to pre-seed a
  fork table (e.g. v15 ring-16 from block 0) via `hardforks` table instead of
  inheriting Monero's upgrade history.

## 2. Networking & Seed Infrastructure — 4/10

**Done:** 2-node loopback sync proven (`--allow-local-ip` +
`--add-exclusive-node`); self-seed footgun removed (empty `get_ip_seed_nodes()`);
ZMQ/RPC/P2P port separation validated; `host-seed-node.bat` + `join-testnet.bat`.

**Needs improving:**
- **No live seeds.** Deploy ≥2 VPS seeds in different providers:
  `host-seed-node.bat` (Windows) or `vendor/monero/utils/systemd/monerod.service`
  (Linux). Open only TCP 48080 (testnet P2P); RPC/ZMQ stay on localhost.
- **Hardcode seeds** once IPs are stable: `get_ip_seed_nodes()` in
  `vendor/monero/src/p2p/net_node.inl` (IP form) or `m_seed_nodes_list` in
  `net_node.h` (DNS form). Then drop `--add-peer` from join instructions.
- **`--allow-local-ip` must not ship in production scripts** — it exists only
  for loopback testing. `join-testnet.bat` currently includes it; gate it
  behind a `--local` flag when real seeds exist.
- **No ban/DoS tuning review** for public exposure (`--ban-list`,
  rate limits, `--limit-rate`). Testnet will get scanned/attacked quickly.

## 3. Mining & Block Production — 7/10

**Done:** in-daemon RandomX solo mining works via URI and `/json_rpc`;
dynamic difficulty verified (1 → 98k under load); `xrl-miner.bat` one-click.

**Needs improving:**
- **No stratum/pool support.** Pools need `monero-wallet-rpc` + a pool server
  (e.g. monero-stratum or p2pool fork) pointed at XRL's RPC. Neither is tested.
- **`start_mining` BUSY quirk:** a tip-holding node reports
  `"synchronized": false` until it syncs *from* a peer, blocking mining starts.
  Workarounds documented (`--offline` for solo, or mine from a synced node),
  but consider patching `core_rpc_server.cpp::on_start_mining` /
  `is_within_compiled_block_hash_area` interplay for smoother solo UX.
- **No mining watchdog** — scripts should detect a wedged miner
  (`mining_status` speed == 0 while active) and restart.

## 4. Wallet, Addresses & Services — 5/10

**Done:** new-prefix wallets generate/sync/refresh; balances track mined
outputs; 10-block unlock enforced; `monero-wallet-rpc.exe` builds.

**Needs improving:**
- **`monero-wallet-rpc` is built but untested** end-to-end (no exchange-style
  integration test: create address → receive → confirm → send).
- **Address book/mnemonic flow** for the new prefix: verify 25-word seed
  restore round-trips on a fresh prefix (test before mainnet keys exist).
- **No view-only / audit wallet runbook** for the genesis account. The genesis
  seed strings (`RandomLite-XRL-*-Genesis`) are published in
  `cryptonote_config.h` comments — anyone can regenerate the genesis address.
  That is fine for a zero-value genesis, but document it explicitly so nobody
  mistakes it for a premine wallet.
- **Subaddress prefix (0x1b6) untested.**

## 5. Build, Release & Packaging — 4/10

**Done:** `build-monero.bat` reproducible on this machine; 30 runtime DLLs
staged beside binaries (portable); `xrl-genesis-tool` wired into CMake.

**Needs improving:**
- **No CI.** Add GitHub Actions: Windows (LLVM-MinGW + vcpkg cache) + Linux
  build of daemon/simplewallet/wallet-rpc/genesis-tool; gate PRs on build +
  a smoke test that starts a regtest node and mines 3 blocks.
- **No installer/zip release packaging** — ship `build\monero-xrl\bin` +
  staged DLLs + the three launcher scripts as a versioned release artifact.
- **Not deterministically reproducible** (build paths, vcpkg versions drift).
  Pin vcpkg baseline + toolchain version in a manifest for binary verification.
- **`build-monero.bat` doesn't pass `-DMANUAL_SUBMODULES=1`** — after the
  submodule flattening, a fresh clone's first configure will fail on the
  submodule check. Add the flag to the bat file (and keep vendored externals
  flattened).

## 6. Security & Hardening — 3/10

**Done:** wallet keys + chain data git-ignored; RPC bound to localhost in all
scripts; restricted-RPC mode available.

**Needs improving:**
- **No external review of consensus diffs.** The diff vs upstream Monero is
  small and auditable (`git diff` against Monero v0.18.1.0 tag): genesis,
  prefixes, emission constants, RPC dispatch macro, seed list. Commission or
  perform a line-by-line review before mainnet.
- **Checkpoints/DNS checkpointing disabled** in all scripts
  (`--disable-dns-checkpoints`). For public testnet, keep enabled once DNS
  exists; add hardcoded height→hash checkpoints after the first few thousand
  blocks to resist deep reorgs.
- **No fuzzing/sanitizer pass** on the fork-diff code paths.
- **Default wallet password `passwordpass123` is hardcoded in scripts** —
  acceptable for testnet, must be removed/parameterized before real value exists.

## 7. Testing & QA — 3/10

**Done:** manual E2E (mine → sync → wallet balance) on fresh chains; 2-node
relay test.

**Needs improving:**
- **No automated test suite runs.** `tests/test_consensus_tdd.cpp` targets the
  quarantined prototype, not the real chain. Port core checks to Monero's
  `core_tests`/functional test harness, or write RPC-driven Python functional
  tests (vendor has `utils/python-rpc`) covering: genesis hash, mining 5 blocks,
  2-node sync, wallet send/receive, reorg of depth 2.
- **No regression guard for the `/json_rpc` dispatch fix** — add a functional
  test asserting `start_mining`/`mining_status`/`stop_daemon` via `/json_rpc`.
- **No soak test** — run a 3-node mesh for 24h before calling testnet public.

## 8. Mainnet Launch Readiness — 4/10

**Done:** mainnet genesis tx + network ID + ports defined and deterministic;
branding is XRL.

**Needs improving:**
- **Mainnet genesis audit pass:** re-derive with `xrl-genesis-tool`, publish the
  seed + resulting hash in the release notes *before* launch so the genesis is
  provably fair (no hidden premine).
- **Premine decision not encoded.** Current emission starts from block reward
  (~17.5 XRL) with zero premine. If a premine/dev fund is intended, it must be
  a transparent genesis output or defined block schedule — decide and document.
- **Mainnet address prefixes still Monero's** (18/19/42). Pick unique mainnet
  prefixes (testnet uses 0x1b4–0x1b6) and verify base58 leading characters.
- **Launch checklist:** seeds deployed → prefixes → genesis published →
  checkpoints seeded → explorer/status page → only then announce.

## 9. Documentation & Ops — 5/10

**Done:** this roadmap; `MONERO_XRL_MIGRATION.md` status section; README with
quickstart; launcher scripts self-document via `--help`.

**Needs improving:**
- **No ops runbook:** seed node monitoring (height drift, peer count), log
  rotation (`--max-log-file-size`), backup/restore of `data.mdb`, upgrade
  procedure.
- **No public status page / block explorer** (even a minimal one: `get_info`
  poller → static HTML).
- **No versioning scheme** for XRL releases (currently Monero's version string
  + commit tag). Define `XRL 0.1.0-testnet` etc.

---

## Suggested execution order

1. **CI build + RPC regression test** (§5, §7) — protects everything else.
2. **Deploy 2 testnet seeds, hardcode them** (§2) → public testnet.
3. **wallet-rpc integration test + 3-node soak** (§4, §7).
4. **RandomX 1440 epoch + hardfork-table decision** (§1) — last consensus
   change window before mainnet.
5. **Security review of fork diff** (§6).
6. **Mainnet prefixes + genesis audit + launch checklist** (§8).

## Quick command reference

| Task | Command |
|---|---|
| Build production | `build-monero.bat` |
| Solo testnet (all-in-one) | `xrl-miner.bat [--threads N]` |
| Host a seed | `host-seed-node.bat [--mainnet]` (admin for firewall) |
| Join hosted testnet | `join-testnet.bat <SEED_IP> [--threads N]` |
| Regenerate genesis | `build\monero-xrl\bin\xrl-genesis-tool.exe "<seed>" <nonce>` |
| 2-node local test | node B: `monerod --testnet --allow-local-ip --add-exclusive-node 127.0.0.1:48080 --p2p-bind-port 58080 --rpc-bind-port 58081 --zmq-rpc-bind-port 58082 --data-dir <dirB>` |
