# Available geometries for VTX

Currently the following geometries are available:

- VTX-CMOS-5layer-staggered-L3mod
- VTX-CMOS-6layer-staggered-L3mod
- VTX-CMOS-6layer-staggered-L3mod-2xL12
- VTX-CMOS-6layer-staggered-L3mod-3xL12
- VTX-CMOS-6layer-staggered-L3mod-3xL12-1_5xL3456
- VTX-CMOS-6layer-staggered-L3mod-3xL12-2xL3456

These contain the latest dimensions of the sensor, individual sensors are implemented with proper gap between them and the layers 4 to 6 have a staggered structure.

Here, L3mod represents layer 3 placed at 69 mm radius instead of 39 mm.
nxL12 represents that the material budgets of the the Layers 1 and 2 are modified to be n times their standard 0.1% X0.
nxL3456 represents that the material budgets of the the Layers 3 to 6 are modified to be n times their standard 0.4% X0.

The *VTX-CMOS-6layer-staggered-L3mod-3xL12-2xL3456* is considered as the current baseline geometry.

For more information on which GTs and basf2 branches to use for different geometries see [online documentation](https://vtxupgrade.pages.desy.de/vtxdvpmt/wg1-performance/geometries-list/).

Old and deprecated geometries:
- VTX-CMOS-5layer
- VTX-CMOS-5layer-L3mod
These were used for the results shown in CDR, but not considered realistic anymore.

Not fully validated:
- VTX-CMOS5-staggered.xml
- VTX-CMOS6.xml
