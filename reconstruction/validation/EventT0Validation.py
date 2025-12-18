#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <input>EvtGenSimRec.root</input>
  <output>EventT0Validation.root</output>
  <contact>Giacomo De Pietro (giacomo.pietro@kit.edu)</contact>
  <description>
  Check the EventT0 performance
  </description>
</header>
"""

import basf2

INPUT_FILENAME = "../EvtGenSimRec.root"
EventT0_Validation_Name = "EventT0Validation.root"


def run():
    """
    Run the EventT0 validation.
    """
    basf2.set_random_seed(1509)
    path = basf2.create_path()

    path.add_module('RootInput', inputFileName=INPUT_FILENAME)
    path.add_module('Gearbox')

    path.add_module('EventT0Validation', RootFileName=EventT0_Validation_Name)

    path.add_module('Progress')

    print(path)
    basf2.process(path)


if __name__ == '__main__':
    run()
