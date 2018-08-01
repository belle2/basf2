"""
---------------------------------------------------------------------------
Script to convert plots from npz format to ASCII - used for the memory testing

> cd ~arich/examples/
> printenv | grep -i belle2
> which basf2
> which python3
> b2code-memoryusage -h
> b2code-memoryusage -m record -i 0.01 -p ARICHStandAlone_memory.npz basf2 -n 1000
 ARICHStandAlone.py -- -b -m | tee ARICHStandAlone_memory.log
> python3 ../utility/scripts/convert_npz_to_ASCII.py --npzfile=ARICHStandAlone_memory.npz --dump
> python3 ../utility/scripts/convert_npz_to_ASCII.py --npzfile=ARICHStandAlone_memory.npz --arrayname total_memory
> root -l ../utility/scripts/convert_npz_ASCII_to_root.C"(\"total_memory.dat\")"

Author: Leonid Burmistrov (Fri Jun 15 14:58:32 JST 2018)
---------------------------------------------------------------------------
"""

import os
import sys
from optparse import Option, OptionValueError, OptionParser
import numpy as np

parser = OptionParser()
parser.add_option('-f', '--npzfile', dest='npzfile', default='ARICHStandAlone_memory.npz', help='Name of the input npz file')
parser.add_option('-a', '--arrayname', dest='arrayname', default='total_memory',
                  help='Name of the array to convert in ASCII format')
parser.add_option('-d', '--dump', action="store_true", dest='dump', default=False, help='Dump list of arrays')
parser.add_option('-p', '--printv', dest='printv', default='', help='Name of the array to print')
(options, args) = parser.parse_args()

npzfile = np.load(options.npzfile)

if(options.dump):
    for myarr in npzfile.files:
        print(myarr)

if(len(options.arrayname) > 0):
    outDatFile = options.arrayname + '.dat'
    np.savetxt(outDatFile, npzfile[options.arrayname], delimiter=' ')

if(len(options.printv) > 0):
    print(npzfile[options.printv])
