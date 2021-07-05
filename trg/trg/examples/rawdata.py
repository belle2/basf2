#!/usr/bin/env python
#
# -----------------------------------------------------------------------------------
#
#                        GDL trigger Unpacker module
#
#    usage : %> basf2 TrgGdlUnpacker.py [input sroot file name]
#
# -----------------------------------------------------------------------------------

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2 as b2

import os
import sys
import re

argvs = sys.argv  # get arg
argc = len(argvs)  # of arg

if argc != 2:
    sys.exit("...py> # of arg is strange.\n 1.rootname\n Exit.")

if argc == 2:
    f_in_root = argvs[1]


# set_log_level(LogLevel.ERROR)
b2.set_log_level(b2.LogLevel.INFO)

b2.use_central_database("TRGGDL_201811")

# input
if f_in_root[-6:] == ".sroot":
    input = b2.register_module('SeqRootInput')
    matchobj = re.search("([^\\/]+)\\.sroot", f_in_root)
    basename = re.sub('\\.sroot$', '', matchobj.group())
if f_in_root[-5:] == ".root":
    input = b2.register_module('RootInput')
    matchobj = re.search("([^\\/]+)\\.root", f_in_root)
    basename = re.sub('\\.root$', '', matchobj.group())

print(f_in_root)
input.param('inputFileName', f_in_root)

anarawdata = b2.register_module('TRGRAWDATA')

anarawdata.param('nwd_2ds', 3939)
anarawdata.param('nwd_3ds', 3939)

anarawdata.param('print_dbmap', True)
anarawdata.param('print_clkcyc_err', True)
anarawdata.param('cc_check', True)
anarawdata.param('print_cc', True)
anarawdata.param('on_2d0', True)
anarawdata.param('on_2d1', True)
anarawdata.param('on_2d2', True)
anarawdata.param('on_2d3', True)
anarawdata.param('on_gdl', True)
anarawdata.param('on_sl5', True)
anarawdata.param('on_sl6', True)
anarawdata.param('on_sl8', False)
anarawdata.param('on_etf', False)
anarawdata.param('on_grl', False)
anarawdata.param('on_nn0', True)
anarawdata.param('on_nn1', True)
anarawdata.param('on_nn2', True)
anarawdata.param('on_nn3', True)
anarawdata.param('on_3d0', True)
anarawdata.param('on_3d1', True)
anarawdata.param('on_3d2', True)
anarawdata.param('on_3d3', True)
anarawdata.param('on_top', True)
anarawdata.param('scale_top', 32)

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(input)
main.add_module(anarawdata)

gen_hist = False
anarawdata.param('histRecord', False)
if gen_hist:
    if not os.path.isdir('rawdatahist'):
        os.mkdir('rawdatahist')

    histo = b2.register_module('HistoManager')
    histo.param("histoFileName", "rawdatahist/raw.%s.root" % basename)
    main.add_module(histo)

b2.process(main)
print(b2.statistics)
