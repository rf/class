# third

`third` is a simple userland thread library. It is capable of preemptive, round
robin scheduling. It implements most of the functionality available in 
`pthreads`.

## Sorting example

The sorting example is called `test_sort.c` and it can be tested as such:

```
make test
./sort
```

I had to make some minor modifications for it to work with my library, which
has a bit of a weird API.

## Consistency with pthreads

`third` isn't completely consistent with `pthreads`. Its creation
functions return pointers to structs rather than operating on pointers to
structs. Many of the functions require the current `third` to be passed in,
which makes the implementation a little bit simpler.

## Detecting whether we interrupted the scheduler

Instead of using E/RIP to detect if we interrupted the scheduler, I instead
use the stack base pointer.  This is a pretty silly way of detecting whether
we interrupted the scheduler, but it does work.  Basically, I just check to see
if RBP is within 10mb of the stored RBP for the scheduler; if it I don't
preempt.

## Mutexes and disabling the scheduler

I have a pretty stupid way of disabling the scheduler while doing mutex
operations, in order to make them atomic. I basically just write a flag that
says "don't preempt". Memory-aligned writes are atomic on x86, so this works;
but it's relatively naive.

## Mailbox functionality

I added a simple mailbox API to my library. It allows circular `box`es to be
created with a certain number of `slot`s. Once a slot has been read, it is
marked as such and can be used as a place for a new message. Reads and writes
to the boxes will block a `third`.
