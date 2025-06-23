##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import basf2
from tracking.path_utils import add_cdc_track_finding

os.environ["OMP_NUM_THREADS"] = "4"  # export OMP_NUM_THREADS=4
os.environ["OPENBLAS_NUM_THREADS"] = "4"  # export OPENBLAS_NUM_THREADS=4
os.environ["MKL_NUM_THREADS"] = "6"  # export MKL_NUM_THREADS=6
os.environ["VECLIB_MAXIMUM_THREADS"] = "4"  # export VECLIB_MAXIMUM_THREADS=4
os.environ["NUMEXPR_NUM_THREADS"] = "6"  # export NUMEXPR_NUM_THREADS=6

basf2.set_log_level(basf2.LogLevel.INFO)
basf2.conditions.prepend_testing_payloads('/work/gdepietro/forCPPCat/localdb/database.txt')

main = basf2.Path()

main.add_module('ProgressBar')
main.add_module('RootInput')
main.add_module('Gearbox')
main.add_module('Geometry')
main.add_module('SetupGenfitExtrapolation')

main.add_module(
        "TFCDC_WireHitPreparer",
        wirePosition="aligned",
        useSecondHits=False,
        flightTimeEstimation="outwards",
        filter="cuts_from_DB",
    )
main.add_module("CDCHitBasedT0Extraction", rejectIfNotTakenFlag=False)

main.add_module('CATFinder')

add_cdc_track_finding(main, skip_WireHitPreparer=True)

basf2.process(main)
print(basf2.statistics)
