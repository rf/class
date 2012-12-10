# third

`third` is a simple userland thread library. It is capable of preemptive, round
robin scheduling. It implements most of the functionality available in 
`pthreads`.

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
