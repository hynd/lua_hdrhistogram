Lua HdrHistogram Library
------------------------

## Overview
HdrHistogram is an algorithm designed for recording histograms of value measurements with configurable precision.  Value precision is expressed as the number of significant digits, providing control over value quantization and resolution whilst maintaining a fixed cost in both space and time.
More information can be found on the [HdrHistogram site](http://hdrhistogram.org/) (which much of the text in this README paraphrases).  This library wraps the [C port](https://github.com/HdrHistogram/HdrHistogram_c).

## Installation

### Prerequisites
* C compiler (GCC 4.7+, Visual Studio 2013, MinGW (Lua 5.1))
* Lua 5.1, Lua 5.2, or LuaJIT
* [CMake (2.8.7+)](http://cmake.org/cmake/resources/software.html)

### CMake Build Instructions
    git clone https://github.com/hynd/lua_hdrhistogram.git
    cd lua_hdrhistogram
    mkdir release
    cd release

    # UNIX
    cmake -DCMAKE_BUILD_TYPE=release ..
    make

## Module

### Example Usage
```lua
require "hdrhistogram"
local hdr = hdrhistogram.new(1,1000000,3)

for i = 10, 1000000, 10 do
  hdr:record(i)
end

local p50 = hdr:percentile(50)
-- p50 == 500223
```

### API Functions

#### new
```lua
require "hdrhistogram"
local hdr = hdrhistogram.new(0,3600000000,3)
```

Import Lua _hdrhistogram_ via the Lua 'require' function. The module is
globally registered and returned by the require function. The _new_ function
takes 3 arguments for lowest trackable value, highest trackable value and
the value precision represented as significant figures.

The example above configures a new histogram to track the counts of observed
integer values between 0 and 3,600,000,000 while maintaining a value precision
of 3 significant digits across that range.  Value quantization within the range
will thus be no larger than 1/1,000th (or 0.1%) of any value. This example
Histogram could be used to track and analyze the counts of observed response
times ranging between 1 microsecond and 1 hour in magnitude, while maintaining
a value resolution of 1 microsecond up to 1 millisecond, a resolution of 1
millisecond (or better) up to one second, and a resolution of 1 second (or
better) up to 1,000 seconds. At its maximum tracked value (1 hour), it would
still maintain a resolution of 3.6 seconds (or better).

### API Methods

#### record
```lua
local success = hdr:record(value)
```

Adds a value to the histogram.

*Arguments*
- value (number) The value to add to the histogram.
*Return*
- True if the value was successfully recorded.

#### min
```lua
local min = hdr:min()
```

*Arguments*
- none

*Return*
- Returns the minimum value stored in the histogram.

#### max
```lua
local max = hdr:max()
```

*Arguments*
- none

*Return*
- Returns the maximum value stored in the histogram.

#### mean
```lua
local mean = hdr:mean()
```

*Arguments*
- none

*Return*
- Returns the calculated mean average of all values in the histogram.

#### stddev
```lua
local stddev = hdr:stddev()
```

*Arguments*
- none

*Return*
- Returns the calculated standard deviation of values in the histogram.

#### percentile
```lua
local p999 = hdr:percentile(pct)
```

Get the value at a given percentile.

*Arguments*
pct (number) Percentile to retrieve.

*Return*
- Returns the value that the given percentage of the overall recorded entries are either smaller than or equivalent to.

#### merge
```lua
dropped = hdr:merge(old)
```

Merges one histogram into another.

*Arguments*
- old (hdrhistogram) A hdrhistogram to merge into the current one.

*Return*
- dropped (number) Number of values dropped during the merge operation.

#### reset
```lua
hdr:reset()
```

Empties the contents of the histogram.

*Arguments*
- none

*Return*
- none

#### memsize
```lua
size = hdr:memsize()
```

Returns the amount of memory allocated to the hdrhistogram in bytes.

*Arguments*
- none

*Return*
size (number) Size in bytes allocated to the histogram object.

#### count
```lua
count = hdr:count()
```

*Arguments*
- none

*Return*
count (number) Total count of all recorded values in the histogram.

## Todo
* Add API for recording values corrected for Co-ordinated Omission.
* Take table of numbers into :record()
* Serialisation
