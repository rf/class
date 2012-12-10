# third

`third` is a simple userland thread library. It is capable of preemptive, round
robin scheduling. It implements most of the functionality available in 
`pthreads`.

There are a few important differences between `third` and `pthreads`.  The 
creation functions usually return pointers rather than intializing a struct.

