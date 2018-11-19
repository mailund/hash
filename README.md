# Various hash tables implemented in C

This repository contains various implementations of hash tables for sets and maps. Each implementation consists of one header and one implementation file, so they are lightweight to add to your own projects. You can find them in the source sub-directories for each of the directories.

To use the tables, just add the source files to your project. All the set implementations have the same name and so do all the map implementations, so you might have to change the names if you need more than one of these.

To cite these implementations, please use [*The Joys of Hashing*](https://amzn.to/2pngZQ0).

![](joys-of-hashing.jpg)

## Implementations

* [Chained hash sets](ChainedHashSet/source) — Hash set with linked lists for conflict resolution.
* [Linear probe hash set](LinearProbeHashSet/source) — Hash set with open addressing linear probes. If you want double hashing instead, you can replace the probe function with the one below, but linear probing is usually faster for larger hash tables because of its cache efficiency.

```c
static uint32_t
p(uint32_t k, uint32_t i, uint32_t m)
{
    uint32_t h1 = k;
    uint32_t h2 = (k << 1) | 1;
    return (h1 + i*h2) & (m - 1);
}
```