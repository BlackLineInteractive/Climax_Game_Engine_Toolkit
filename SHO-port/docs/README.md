# SHO-port

The gameplay half of the port. `climax-core` reads the retail data and
`climax-game` holds the systems that were transcribed out of the executable -
the boot sequence, the front end, the scene queue, the camera and zone links.
This directory is where everything above that goes: Travis, the monsters,
items, combat, the puzzles.

Right now it holds almost nothing, on purpose. The first attempt at those
systems was written without the 7698 decompiled functions under
`decomp/sho_sles_out/` - real class names and constructor layouts, invented
behaviour, and 69 message names the game does not have. It was audited on
2026-10-12 and moved, unbuilt, to `archive/gemini-unverified/`; `AUDIT.md`
there goes through it file by file. What is left here is the generated
message table (`include/SHO/Core/EventIds.h`), `src/Main.cpp` (the level
loop, with its walk harness labelled as such) and these documents.

Code comes back one recovered function at a time, with its `SLES_551.47`
address beside it.

So each document below is split the same way:

**Recovered** - an address in `SLES_551.47`, or a query over `SH.ARC` that
anyone can re-run. If a number appears under this heading it came out of the
retail build.

**Not recovered** - what the archived code did instead. None of it is built
any more; it is listed so the next reading knows which guesses to check, not
which to keep.

Nothing should move from the second heading to the first without an address or
a command line beside it.

| | |
|---|---|
| [PLAYER.md](PLAYER.md) | `CPlayerBehaviour` - the message wiring, and the state machine that has not been read yet |
| [ENEMIES.md](ENEMIES.md) | `CEnemyBehaviour` and the eight monsters derived from it |
| [ITEMS.md](ITEMS.md) | the item table - 164 rows straight out of the archive, with damage, durability, clip sizes and sounds |
| [GRAPPLE.md](GRAPPLE.md) | `CFMAController` and `cThreatController` - the struggle and the threat meter |

## Where the facts come from

Three sources, in descending order of trust.

**The retail executable, `game-iso/SHO/SLES_551.47`.** Stripped, so a function
is `FUN_00136CC8` and a global is `DAT_006B4C50`. `decomp/sho_sles_out/ghidra_clean/` holds
the whole thing decompiled with the Emotion Engine processor module - see
`docs/TODO.md` §6c for why the stock MIPS module produces garbage here.

**The archive, `game-iso/SHO/SH.ARC`.** Every object a designer placed is in
there with its properties, which means the item table, the enemy tuning and the
threat settings are data rather than code. `tools/dump_class.py` pulls one
class out whole.

**Ghost Rider, `SLES_543.17`.** Same engine, unstripped, so a routine has its
real name. It settles what a system *is*; it does not settle what Origins does
with it, because the two games do not share every routine. `docs/README.md`
carries the standing caution.

## Regenerating what is generated

    python3 tools/event_ids.py
    python3 tools/dump_class.py game-iso/SHO/SH.ARC CInventoryItemDef \
        --json docs/generated/sho_inventory.json

The first writes `docs/generated/sho_event_ids.json` and
`include/SHO/Core/EventIds.h`; the second writes the item table these documents
quote. Neither is edited by hand.
