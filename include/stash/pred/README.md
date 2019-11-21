# Static Predecessor Data Structures

These are implementations of predecessor data structures over sorted integer arrays. All of them share the common template parameters
* `array_t` - the type of the underlying integer array that must provide index access using `[]` and a `size()` method, e.g., `std::vector`) - and
* `item_t` - the item type, which is expected to support all common integer operators.

The methods provided are `predecessor` and `successor`, which report the _position_ (not value) of the predecessor or successor, respectively, of a given value in the input sequence. Note that the input sequence is _not_ copied and must therefore remain in memory after construction.

## Overview

A brief overview over the relevant implementations is given in the following table:

| Class | Description |
| --- | --- |
| `binary_search` | Stores no data but simply performs a binary search on the input. |
| `binary_search_cache` | Like `binary_search`, but proceeds with linear search once the search interval becomes smaller than `m_cache_num` (template parameter). If the parameter is chosen well (the default asssumes cache lines of 512 bytes), this is practically faster than binary search by some percents. |
| `sample` | Samples every `m_alpha`-th value (template parameter) and splits the binary search up into two binary searches on smaller memory regions, which are done using `binary_search_cache`. Very low memory profile, but only marginal speed improvements compared to binary searches. |
| `index` | Indexes the sequence based on the universe defined by `item_t` by saving a search interval of size at most 2 to the `m_lo_bits`-th power (template parameter) for each possible combination of high bits (`8 * sizeof(item_t) - m_lo_bits`). The interval is then searched using `binary_search_cache`. This implementation yields very good time/space trade-offs. |
| `index_compact` | Same as `index`, but compresses the stored the interval borders. Saves a lot of RAM, but comes at the cost of slower queries. |
| `rank` | Constructs a bit vector with constant-time rank support for the given input sequence. Fastest implementation, but the required RAM depends directly on the difference between the largest and smallest value in the input sequence. |

## Usage Example

Here's an example for using `index`:

```
// a simple array - values MUST be in ascending order
std::vector<uint8_t> array = { 2, 4, 19, 37, 55, 78, 98, 102, 147, 200 };

// specify the type for our index
// in this example, we index the upper 3 bits of each value
using index = stash::pred::index<decltype(array), uint8_t, 3>;

// construct the index
index idx(array);

// send a predecessor query
auto r = idx.predecessor(128);

// print the result (should be 102)
std::cout << "result: " << uint(array[r.pos])
          << ", expected: 102" << std::endl;
```
