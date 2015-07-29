Most files in this directory where taken directly from
https://github.com/gvanas/KeccakCodePackage and are part of the reference
implementation of the Keccak Hash function family (FIPS202 draft for SHA3 hash
functions, http://keccak.noekeon.org/)

The only changes are whitespace modifications in the headers to adhere to the
Belle2 style requirements. No functional changes were done. The corresponding
files in the official repository are (as of 2015-01-15):

brg_endian.h -> KeccakCodePackage/Common/brg_endian.h
KeccakDuplex.c -> KeccakCodePackage/Constructions/KeccakDuplex.c
KeccakDuplex.h -> KeccakCodePackage/Constructions/KeccakDuplex.h
KeccakF-1600-64.macros -> KeccakCodePackage/SnP/KeccakF-1600/Optimized/KeccakF-1600-64.macros
KeccakF-1600-interface.h -> KeccakCodePackage/SnP/KeccakF-1600/KeccakF-1600-interface.h
KeccakF-1600-opt64.c -> KeccakCodePackage/SnP/KeccakF-1600/Optimized64/KeccakF-1600-opt64.c
KeccakF-1600-opt64-settings.h -> KeccakCodePackage/SnP/KeccakF-1600/Optimized64/u6/KeccakF-1600-opt64-settings.h
KeccakF-1600-unrolling.macros -> KeccakCodePackage/SnP/KeccakF-1600/Optimized/KeccakF-1600-unrolling.macros
KeccakHash.c -> KeccakCodePackage/Modes/KeccakHash.c
KeccakHash.h -> KeccakCodePackage/Modes/KeccakHash.h
KeccakSponge.c -> KeccakCodePackage/Constructions/KeccakSponge.c
KeccakSponge.h -> KeccakCodePackage/Constructions/KeccakSponge.h
SnP-FBWL-default.c -> KeccakCodePackage/SnP/SnP-FBWL-default.c
SnP-FBWL-default.h -> KeccakCodePackage/SnP/SnP-FBWL-default.h
SnP.h -> KeccakCodePackage/SnP/SnP.h
SnP-interface.h -> KeccakCodePackage/SnP/KeccakF-1600/Optimized64/SnP-interface.h
SnP-Relaned.h -> KeccakCodePackage/SnP/SnP-Relaned.h

An simple user interface cam be found in Hash.h and Hash.cc
