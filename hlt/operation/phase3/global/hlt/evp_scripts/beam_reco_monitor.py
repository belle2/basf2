#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2
from softwaretrigger import constants
from softwaretrigger.processing import setup_basf2_and_db, start_path, finalize_path, add_hlt_processing

args = setup_basf2_and_db()

path = start_path(args, location=constants.Location.hlt)
add_hlt_processing(path, run_type=constants.RunTypes.beam,
                   softwaretrigger_mode=constants.SoftwareTriggerModes.monitor)
finalize_path(path, args, location=constants.Location.hlt)

basf2.print_path(path)
basf2.process(path)
