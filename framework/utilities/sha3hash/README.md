Most files in this directory were taken directly from https://github.com/gvanas/KeccakCodePackage and are part of the reference implementation of the Keccak Hash function family (FIPS202 draft for SHA3 hash functions, http://keccak.noekeon.org/).

The only changes applied are whitespace modifications in the headers to adhere to the basf2 style requirements. No functional changes were done. The corresponding files in the official repository are listed in `update_hints.sh` (as of revision `e39f89a` from 2016-09-07).

A simple user interface can be found in `Hash.h` and `Hash.cc`.
