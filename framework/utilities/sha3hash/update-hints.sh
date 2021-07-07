# Steps that were taken for the last update. Used to work as presented.
# However the folder structure and naming of the files changed from last time,
# so it is probably necessary to make adjustments in the next update as well.

git clone https://github.com/gvanas/KeccakCodePackage.git

# General purpose headers
cp KeccakCodePackage/Common/align.h . &&
cp KeccakCodePackage/Common/brg_endian.h . &&

# Implementation
cp KeccakCodePackage/SnP/KeccakP-1600/Optimized64/u6/KeccakP-1600-opt64-config.h . &&
cp KeccakCodePackage/SnP/KeccakP-1600/Optimized/KeccakP-1600-64.macros . &&
cp KeccakCodePackage/SnP/KeccakP-1600/Optimized/KeccakP-1600-unrolling.macros . &&
cp KeccakCodePackage/SnP/SnP-Relaned.h . &&
cp KeccakCodePackage/SnP/KeccakP-1600/Optimized64/KeccakP-1600-opt64.c . &&

# Abstraction
cp KeccakCodePackage/SnP/KeccakP-1600/Optimized64/KeccakP-1600-SnP.h . &&
cp KeccakCodePackage/Constructions/KeccakSponge.h . &&
cp KeccakCodePackage/Constructions/KeccakSponge.inc . &&
cp KeccakCodePackage/Constructions/KeccakSponge.c . &&
cp KeccakCodePackage/Modes/KeccakHash.h . &&
cp KeccakCodePackage/Modes/KeccakHash.c . &&

rm -rf KeccakCodePackage

# Clean up white spaces
fixstyle
