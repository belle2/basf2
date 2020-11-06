#!/usr/bin/env python
#
# -----------------------------------------------------------------------------------
#
#                        GDL trigger Unpacker module
#
#    usage : %> basf2 TrgGdlUnpacker.py [input sroot file name]
#
# -----------------------------------------------------------------------------------
from basf2 import *

import sys
import re

argvs = sys.argv  # get arg
argc = len(argvs)  # of arg

if argc != 2:
    sys.exit("...py> # of arg is strange.\n 1.rootname\n Exit.")

if argc == 2:
    f_in_root = argvs[1]


# set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)

use_central_database("TRGGDL_201811")

# input
if f_in_root[-6:] == ".sroot":
    input = register_module('SeqRootInput')
    matchobj = re.search("([^\/]+)\.sroot", f_in_root)
    basename = re.sub('\.sroot$', '', matchobj.group())
if f_in_root[-5:] == ".root":
    input = register_module('RootInput')
    matchobj = re.search("([^\/]+)\.root", f_in_root)
    basename = re.sub('\.root$', '', matchobj.group())

print(f_in_root)
input.param('inputFileName', f_in_root)

anarawdata = register_module('TRGRAWDATA')

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
main = create_path()

# Add modules to main path
main.add_module(input)
main.add_module(anarawdata)

gen_hist = False
anarawdata.param('histRecord', False)
if gen_hist:
    if not os.path.isdir('rawdatahist'):
        os.mkdir('rawdatahist')

    histo = register_module('HistoManager')
    histo.param("histoFileName", "rawdatahist/raw.%s.root" % basename)
    main.add_module(histo)

process(main)
print(statistics)
