#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import sys

import basf2
from ROOT.Belle2 import KLMTimeAlgorithm
from klm_calibration_utils import get_time_pre_collector_path
from prompt.calibrations.caf_klm_time import get_collector
from caf.framework import CAF, Calibration, Collection
from caf import backends

basf2.set_log_level(basf2.LogLevel.INFO)

input_files = sys.argv[1:]

# Create KLM time calibration.
# To run on data, add global tags using cal_klm.use_central_database()
# and, if necessary, cal_klm.use_local_database(). The same chain must be
# set up for collector using collection_cdst.use_central_database() and
# collection_cdst.use_local_database(). Also disable MC setting for
# the algorithm by commenting the line algorithm.setMC(True) or changing
# True to False. The raw CDST format should be turned on in
# get_time_pre_collector_path().
algorithm = KLMTimeAlgorithm()
algorithm.setMC(True)
algorithm.setMinimalDigitNumber(0)
cal_klm = Calibration(name='KLMTime', algorithms=algorithm)

coll_cdst = get_collector('hlt_mumu', 'klmTime')
rec_path_cdst = get_time_pre_collector_path(muon_list_name='klmTime', mc=True)
collection_cdst = Collection(collector=coll_cdst,
                             input_files=input_files,
                             pre_collector_path=rec_path_cdst)
cal_klm.add_collection(name='cdst', collection=collection_cdst)

# Create and run calibration framework.
framework = CAF()
framework.backend = backends.LSF()
framework.add_calibration(cal_klm)
framework.run()
