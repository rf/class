# third

**Russ Frank**

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

## Sorting questions

Since I didn't write the sort example myself, here are my answers to the
questions:

1. Well, this algorithm essentially consists of each thread trying to get a
   chance to perform a swap. Once the thread was able to do a swap, we want to
   now allow a different thread to swap; if we were to run around the loop
   again, there would be nothing to do. So, we yield, with the hope that some
   other thread will perform a swap so we don't waste time the next time we
   pop around that loop.

2. The virtual timer I'm using doesn't appear to increment when a system
   call is in progress; so if that sleep is left in, I never preempt because
   the interrupt is never fired.

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

## Re: extra credit COW idea

I think this is a really fascinating idea. However, I don't think it's
appropriate to implement it with a SIGSEGV handler; this sounds dangerous. Also,
it'd require a pretty ugly interface to work.  Alternatively, a disassembler
could be used to perform surgery on the running code to place in a new address.

It seems like this would be much safer to implement at the kernel level, where
we have the benefit of the paging hardware's translation. COW could be
implemented between threads in exactly the same way it's implemented between
processes. A new system call could be added called `create_cow_reference()` or
so which would return a new address pointing to the same data which would be
COW optimized.

However, I'd also argue that this is rather easy to implement by convention in
a user's program. Using my mailbox code, it's fairly easy to shuffle pointers
between two `third`s. Then, the author of said program would decree that once a
pointer is shuffled across, it is the responsibility of the receiver to deal
with it. This is heavily application dependent, though.
