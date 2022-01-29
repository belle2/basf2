# 1. Compilation

## 1-1. PCIe40 software
Download basf2 and checkout this branch.
`$ git checkout feature/BIIDAQ-174-daq-software-for-pcie40`

Download submodules for PCIe40.( The version of a sub-module is fixed. So it might be old. )
`$ git submodule update --init`

Compile PCIe40 library( See REAEME.md on https://stash.desy.de/projects/BIDU/repos/software/browse ).

## 1-2. DAQ software
If you don't send data to eb0, please define NO_SEND in des_ser_PCIe40_main.cc .

`#define NOT_SEND`

Compile DAQ software.

`$ cd daq/pcie40`

`$ make`

# 2. Read out data from PCIe40
## 2-1. start DAQ program 
`$ ./des_ser_PCIe40_main <node ID:0x********>`

* If you are not on the PCs below, please just use 0x0 as a node ID.
** rtop1    : 0x3000001
** rtop2    : 0x3000002
** rklm1    : 0x7000001

Then, send triggers.
`ttd? $ trigft -<FTSW ID> <trigger type> <rate(kHz)> <# of triggers>`

## 2-2. If you want to receive data from des_ser_PCIe40_main and record a .sroot/root file
You need to set # of sender threads to 1.

In des_ser_PCIe40_main.cc, 
`#define NUM_SENDER_THREADS 5` -> 1

- First you need to compile basf2

`$ cd release`

`$ scons -D`

- Edit and run a python script.

`Script : release/daq/rawdata/examples/Recv2Root.py`

If you are running the program on rklm1, the following is fine.

`reader.param('PortFrom', 30001)`

`reader.param('HostNameFrom', 'rklm1')`

.sroot -> SeqRootOutput
.root  -> RootOutput

`dump = register_module('SeqRootOutput')`

`dump.param('outputFileName', 'root_output.sroot')`

Run the script together with des_ser_PCIe40_main

`$ basf2 Recv2Root.py `



