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