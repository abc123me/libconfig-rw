# libconfig read/writing tool

A simple tool for reading / writing to libconfig files

I needed a way to modify these files via PHP, so I made this.

## Building

Requires `gcc`, `make`, `pkgconfig`, and `libconfig`

Unit tests available in `tests.bats` and uses `bats` with `bats-assert`

The first unit test is to perform a clean build, so you can just run those

Or you can simply run `make`

## Usage:

```
Usage: ./main <filename> <mode> [...]
Modes:
  r/read  - reads configuratiuon values
            expects: <type> <key> [default]
  w/write - sets configuration value
            expects: <type> <key> <val>
  t/type  - reads configuration type
            expects: <key>
  delete  - deletes a configuration value
            expects: <key>
Accepted types:
  int/i, int64/l, float/f, bool/b, string/s, auto/a
Examples:
  Write string "test" to "example.cfg":
    input:   ./main example.cfg w string test "hello world"
    returns: 0
  Read string "test" from "example.cfg":
    input:   ./main example.cfg r string test
    outputs: hello world
    returns: 0
```
