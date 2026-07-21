*This project has been created as part of the 42 curriculum by dbaltaza.*

# Codexion

## Description

Codexion is a concurrency simulation — a variant of the classic *dining
philosophers* problem. A group of **coders** share a limited number of **USB
dongles** that they need in order to compile their code. Each coder is a thread
that cycles through the same life:

1. **acquire two dongles** (its own and the neighbour's),
2. **compile** (needs both dongles),
3. **release the dongles**,
4. **debug**, then **refactor** (needs nothing).

A coder that goes too long without compiling **burns out**. A separate
**monitor** thread watches every coder and logs a burnout within 10 ms of the
instant it happens.

The simulation stops when **either**:

- any coder burns out, **or**
- every coder has compiled at least `number_of_compiles_required` times.

## Instructions

Build:

```
make
```

Run:

```
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
            time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

- All 8 arguments are **mandatory**.
- Times are in **milliseconds**.
- `scheduler` is either `fifo` or `edf`. Any other value (and any negative /
  non-integer argument) is rejected.

Example:

```
./codexion 5 800 200 100 100 3 10 edf
```

Log lines are printed as `<timestamp_ms> <coder_id> <state>`, where the
timestamp is milliseconds since the simulation started:

```
1478 3 has taken a dongle
1478 3 is compiling
1683 3 is debugging
1788 3 is refactoring
306 1 burned out
```

Other targets: `make clean`, `make fclean`, `make re`.

## Resources

- POSIX threads documentation (`man pthread_create`, `man pthread_cond_wait`,
  `man pthread_mutex_lock`, `man gettimeofday`).
- The dining-philosophers problem for the core deadlock / starvation reasoning.
- Binary heap theory (array-backed, sift-up / sift-down) for the hand-rolled
  priority queue.

**How AI was used:** an AI assistant (Claude) was used strictly as a *mentor*,
not an author. It asked guiding questions, explained concurrency concepts
(cooldown races, missed wake-ups, EDF starvation), reviewed the code for norm
issues and race conditions, and pointed at bugs — while I wrote the
implementation myself. Two mechanical pieces (the `heap_pop` sift-down body and
this README) were written with its help; every design decision and the whole
scheduler integration were done and understood by me.

## Blocking cases handled

- **Deadlock (circular wait):** a coder never holds one dongle while waiting for
  the other. Dongles are taken **atomically** — a coder only proceeds when
  *both* of its dongles are free, decided under a single mutex, so the
  hold-and-wait condition never occurs.
- **Cooldown race (missed wake-up):** a released dongle stays unavailable for
  `dongle_cooldown` ms. Waiters block on `pthread_cond_timedwait` with a
  per-iteration absolute deadline, so a dongle that becomes free *during* the
  wait is always retried instead of the thread sleeping forever.
- **Starvation:** requests are ordered by a hand-rolled binary heap. A waiting
  coder only compiles when it is at the **top** of the queue, so no coder is
  overtaken indefinitely — under EDF the coder closest to burning out is served
  first.
- **Single coder (N = 1):** with only one dongle a lone coder can never compile,
  so it terminates cleanly by burning out (it never hangs).
- **Burnout precision:** the monitor sleeps until the *next* candidate burnout
  time (not a fixed poll), so a burnout is always logged within 10 ms.
- **Log / death interleaving:** all logging is serialised through a dedicated
  print mutex, so lines never interleave and the burnout line always appears
  intact.

## Thread synchronization mechanisms

- **One state mutex (`lock`)** protects all shared simulation state: dongle
  availability, the priority queue, per-coder counters and the `stop` flag.
  Every read/write of shared state happens under it, so there are no data races.
- **One print mutex (`print_lock`)** serialises log output independently of the
  state mutex. Lock ordering is always `lock → print_lock`, which rules out a
  lock-ordering deadlock between them.
- **One condition variable (`avail`)** signals dongle-availability and stop
  events. Coders wait on it with `pthread_cond_timedwait`; a release does
  `pthread_cond_broadcast`, and every waiter re-checks its predicate on wake-up
  (no assumptions on a bare signal).
- **Priority queue (binary heap):** a single array-backed heap serves both
  policies through one comparator — the key is the arrival sequence for **FIFO**
  and `last_compile_start + time_to_burnout` for **EDF**, with the coder id as a
  deterministic tie-breaker.
- **Threads:** one thread per coder plus one monitor thread; all are joined
  before cleanup, and every mutex / condition variable is destroyed and every
  allocation freed on exit.
