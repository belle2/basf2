#!/usr/bin/env python
# -*- coding: utf-8 -*-

# STEP 3:
# Millepede II alignment of combined (IP+cosmics) sample
#
# This will output /alignment/examples/computedVXDalignment.xml
# which is linked by step 4a/4b to check alignment on tracks

import sys
from basf2 import *
from alignment_tools import *

# Set the log level to show only warning, error and, fatal messages
set_log_level(LogLevel.WARNING)
release = str(os.getenv('BELLE2_LOCAL_DIR')) + '/'

# -------------------------------------------------
#          IMPORTANT PROCESSING SETTINGS
# -------------------------------------------------

# NOTE: You need a separated (from main xml) file for playing with alignment
alignment = release + 'alignment/data/misalignmentVXD.xml'
# Final sum of misalignment + alignment correction ... should give somehing like ideal alignment hopefully
output_alignment = release + 'alignment/examples/computedVXDalignment.xml'

# Load parameters from xml
gearbox = register_module('Gearbox')

# Create geometry
geometry = register_module('Geometry')

# 1 event
oneevent = register_module('EventInfoSetter')

# Millepede II alignment ... Millepede can be run without basf2
mp2 = register_module('MillepedeIIalignment')
mp2.param('resultXmlFileName', 'VXDcomputed_alignment_corrections.xml')
mp2.param('steeringFileName', 'VXDAlignmentSteering.txt')

# Display progress of processing
progress = register_module('Progress')

# -----------------------------------------------
#               Path construction
# -----------------------------------------------

main = create_path()
main.add_module(oneevent)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(mp2)
# main.add_module(display)
main.add_module(progress)
process(main)

# ------------------------------------------------
#  Sum initial and computed alignment parameters
# ------------------------------------------------
# You can run this together with Millepede from command line:
#
# Create/copy/use existing steering file, set correct path to binary file.
# Then just call pede:
#
#      pede steering_file.txt
#
# The result will be in millepede.res file (in execution path)
# Now just start basf2 as python interpreter (with no arguments)
# and type:
#
#      from alignment_tools import *
#      write_alignment( sum_xmltxt_alignment( 'your_initial_alignment.xml', 'millepede.res' ) )
#
write_alignment(sum_xmltxt_alignment(alignment_xml_path=alignment,
                alignment_txt_path='millepede.res'), output_alignment)

print statistics
