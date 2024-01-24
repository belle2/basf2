#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import os

# ...suppress messages and warnings during processing...
b2.set_log_level(b2.LogLevel.ERROR)
b2.set_random_seed(0)

# ...Register modules...
eventinfosetter = b2.register_module('EventInfoSetter')
gdltrg = b2.register_module('TRGGDL')

# ...EventInfoSetter...
eventinfosetter.param({'evtNumList': [5], 'runList': [1]})

# ...EventInfoSetter...
gdltrg.param('DebugLevel', 1)
gdltrg.param('ConfigFile',
             os.path.join(os.environ['BELLE2_LOCAL_DIR'],
                          "trg/gdl/data/ftd/0.01/ftd_0.01"))

# ...Create paths...
main = b2.create_path()

# ...Add modules to paths...
main.add_module(eventinfosetter)
main.add_module(gdltrg)

# ...Process events...
b2.process(main)

# ...Print call statistics...
print(b2.statistics)
