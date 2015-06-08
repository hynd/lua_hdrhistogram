require "hdrhistogram"
require "string"

local hdr = hdrhistogram.new(1,1000000,3)

local expected_min    = 0
local expected_max    = 0
local expected_p50    = 0

assert(hdr:min() == expected_min, string.format("incorrect min value expected: %d, received %d", expected_min, hdr:min()))
assert(hdr:max() == expected_max, string.format("incorrect max value expected: %d, received %d", expected_max, hdr:max()))
assert(hdr:percentile(50) == expected_p50, string.format("incorrect p50 value expected: %d, received %d", expected_p50, hdr:percentile(50)))

for i = 10, 1000000, 10 do
  hdr:record(i)
end

local expected_min    = 10
local expected_max    = 1000447
local expected_p50    = 500223

assert(hdr:min() == expected_min, string.format("incorrect min value expected: %d, received %d", expected_min, hdr:min()))
assert(hdr:max() == expected_max, string.format("incorrect max value expected: %d, received %d", expected_max, hdr:max()))
assert(hdr:percentile(50) == expected_p50, string.format("incorrect p50 value expected: %d, received %d", expected_p50, hdr:percentile(50)))


print("OK")
