# 1. Compilation

## 1-1. PCIe40 software
Download and compile PCIe40 library
$ cd daq/pcie40
$ ./start_pcie40_threads.sh

## 1-2. DAQ software
If you want to use ZMQ for data transfer in a process:
`#define USE_ZMQ`
You need to setup b2setup to use zmq libray in external.

If you use readout DAQ in recl3(ECL+ECLTRG):
`#define SPLIT_ECL_ECLTRG`

If you don't send data to eb0, please define NO_SEND in des_ser_PCIe40_main.cc .
`#define NOT_SEND`

Compile DAQ software.

`$ cd daq/pcie40`

`$ make`

# 2. Read out data from PCIe40
## 2-1. start DAQ program 
`$ ./des_ser_PCIe40_main <node ID:0x********>`

* If you are not on the PCs below, please just use 0x99000001 as a node ID.
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
