# Queue [![Build Status](https://travis-ci.com/SMFSW/Queue.svg?branch=master)](https://travis-ci.com/SMFSW/Queue)

Queue handling library (designed on Arduino)

This library was designed for Arduino, yet may be compiled without change with gcc for other purposes/targets

Queue class has since start been called `Queue`. Unfortunately, on some platforms or when using FreeRTOS, Queue is already declared.
For compatibility purposes, `Queue` class has been renamed to `cppQueue`. Sorry for the inconvenience...

## Usage

- Declare a cppQueue instance `(uint16_t size_rec, uint16_t nb_recs=20, QueueType type=FIFO, overwrite=false)` (called `q` below):
  - `size_rec` - size of a record in the queue
  - `nb_recs` - number of records in the queue
  - `type` - Queue implementation type: _FIFO_, _LIFO_
  - `overwrite` - Overwrite previous records when queue is full if set to _true_
- Push stuff to the queue using `q.push(void * rec)`
  - returns `true` if successfully pushed into queue
  - returns `false` is queue is full
- Pop stuff from the queue using `q.pop(void * rec)` or `q.pull(void * rec)`
  - returns `true` if successfully popped from queue
  - returns `false` if queue is empty
- Peek stuff from the queue using `q.peek(void * rec)`
  - returns `true` if successfully peeked from queue
  - returns `false` if queue is empty
- Drop stuff from the queue using `q.drop(void)`
  - returns `true` if successfully dropped from queue
  - returns `false` if queue is empty
- Peek stuff at index from the queue using `q.peekIdx(void * rec, uint16_t idx)`
  - returns `true` if successfully peeked from queue
  - returns `false` if index is out of range
  - warning: no associated drop function, not to use with `q.drop`
- Peek latest stored from the queue using `q.peekPrevious(void * rec)`
  - returns `true` if successfully peeked from queue
  - returns `false` if queue is empty
  - warning: no associated drop function, not to use with `q.drop`
  - note: only useful with FIFO implementation, use `q.peek` instead with a LIFO
- Other methods:
  - `q.IsInitialized()`: `true` if initialized properly, `false` otherwise
  - `q.isEmpty()`: `true` if empty, `false` otherwise
  - `q.isFull()`: `true` if full, `false` otherwise
  - `q.sizeOf()`: queue size in bytes (returns 0 in case queue allocation failed)
  - `q.getCount()` or `q.nbRecs()`: number of records stored in the queue
  - `q.getRemainingCount()`: number of records left in the queue
  - `q.clean()` or `q.flush()`: remove all items in the queue

## Notes

- Interrupt safe automation is not implemented in the library. You have to manually disable/enable interrupts where required.
No implementation will be made as it would be an issue when using `peek`/`drop` methods with LIFO implementation:
if an item is put to the queue through interrupt between `peek` and `drop` calls, the `drop` call would drop the wrong (newer) item.
In this particular case, dropping decision must be made before re-enabling interrupts.

## Examples included

- [SimpleQueue.ino](examples/SimpleQueue/SimpleQueue.ino): Simple queue example (both LIFO FIFO implementations can be tested)
- [PointersQueue.ino](examples/PointersQueue/PointersQueue.ino): Queue of string pointers for string processing
- [QueueDuplicates.ino](examples/QueueDuplicates/QueueDuplicates.ino): Simple test to test queue duplicates before pushing to queue
- [QueueIdxPeeking.ino](examples/QueueIdxPeeking/QueueIdxPeeking.ino): Simple test to test queue index picking
- [RolloverTest.ino](examples/RolloverTest/RolloverTest.ino): Simple test to test queue rollover (for lib testing purposes mainly)
- [LibTst.ino](examples/LibTst/LibTst.ino): flexible test (for lib testing purposes mainly)

## Documentation

Doxygen doc can be generated using "Doxyfile".

See [generated documentation](https://smfsw.github.io/Queue/)

## Release Notes

See [release notes](ReleaseNotes.md)

## See also

[cQueue](https://github.com/SMFSW/cQueue) - C implementation of this library
