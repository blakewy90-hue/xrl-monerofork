# Monero to XRL migration

This workspace now contains upstream Monero at `vendor/monero`, pinned to:

`3d3920d7487b5df7ac388b6b8577fd04d505885f`

The imported source provides the real cryptographic implementations, including
Keccak, curve operations, key derivation, RingCT, CLSAG, Bulletproofs+, wallet
serialization, LMDB storage, and RandomX integration. The original `src/`
prototype remains separate until its simplified types and validation paths are
replaced by upstream types.

The active XRL targets now compile Monero's Keccak and BLAKE2b implementations
through a small Windows compatibility bridge. This does not make the prototype
wallet, transaction, or PoW paths production-safe: those paths still need to be
migrated to Monero's canonical types and verification APIs.

The following XRL fundamentals are applied in the imported Monero configuration:

- 150-second target block time
- 10-block mined-money unlock window
- 300,000-byte full-reward zone
- 0.6 XRL tail-emission target constant
- RandomLite network name and ports 28080/28081/28082
- 12 decimal atomic units (unchanged from upstream)

## Required consensus work

The requested 1,440-block RandomX seed epoch cannot be configured by changing a
single constant: upstream RandomX seed scheduling requires a power-of-two epoch
size and currently defaults to 2,048 blocks. Supporting 1,440 requires a
deliberate change to the seed-height algorithm and a new consensus test vector.

The requested exact ring size 16, XRL address prefixes, genesis block, custom
LWMA-3 difficulty, and XRL emission curve also require changes in the imported
consensus and wallet code. They must be implemented together with canonical
serialization and a regenerated genesis block; the old prototype's structs and
placeholder proofs cannot be used as an adapter.

Until that migration is complete, the original binaries are not production
cryptocurrency software and must not share a network with the imported Monero
code.

---

## Migration status (2026-09-04)

The following migration items are now **complete** and verified on a fresh
testnet chain (genesis `bc8210c0...`, height 5+):

- **Distinct genesis blocks**: deterministic `xrl-genesis-tool` generates
  reproducible XRL coinbase transactions for mainnet/testnet/stagenet from a
  published seed. Applied to `cryptonote_config.h` for all three networks.
- **XRL address prefixes**: testnet addresses now use prefix `0x1b4` (standard),
  `0x1b5` (integrated), `0x1b6` (subaddress). Verified: new testnet address
  starts with `X7M2ioWwqi...` (distinct from Monero).
- **Ring size 16**: enforced at HF v15 via `get_min_ring_size()`/`get_max_ring_size()`
  in `wallet2.cpp` and consensus mixin rules in `blockchain.cpp`.
- **Emission / difficulty**: 150s target, 720-block LWMA window, 0.6 XRL tail
  emission already set in the vendored `cryptonote_config.h`.
- **Branding**: binary reports `RandomLite XRL` instead of `Monero 'Fluorine Fermi'`.
- **JSON-RPC mining methods**: `start_mining`, `stop_mining`, `mining_status`,
  `stop_daemon`, `save_bc` now dispatched via `/json_rpc` (new `MAP_JON_RPC_IF` macro).
- **Seed nodes**: hardcoded seed list intentionally left EMPTY. A loopback seed
  was tried and reverted: a node on the default port handshakes with itself,
  never reaches "synchronized", and `start_mining` stays BUSY. For local
  multi-node testing use `--allow-local-ip` on every node plus
  `--add-exclusive-node 127.0.0.1:<p2p-port>` on the joining node (verified:
  2-node sync + live block relay both directions). Production seeds go in
  `net_node.h` once provisioned.
- **Known upstream quirk**: the node holding the chain tip stays
  `"synchronized": false` until it catches up *from* a peer, and `start_mining`
  returns BUSY meanwhile. Mine from the joining/synced node, or use `--offline`
  for single-node solo mining.
- **wallet-rpc**: `monero-wallet-rpc.exe` builds and is produced by the build.
- **Portable binaries**: `build-monero.bat` stages all 30 runtime DLLs next to the
  binaries; `monerod.exe --version` runs with a clean PATH.
- **Stub tree quarantined**: top-level `CMakeLists.txt` prototype targets are
  disabled by default (`XRL_BUILD_PROTOTYPE=OFF`); `build-monero.bat` is the
  authoritative production build.

### Still open for a full mainnet launch
- Real production seed node hostnames/IPs (currently loopback dev seeds).
- Mainnet genesis coinbase is generated but mainnet keys should be finalized in an
  audit pass with the seed published.
- The `src/` prototype is kept for reference only; do not use its binaries.
- RandomX 1440-block seed epoch (non-power-of-two) remains a hard consensus change.