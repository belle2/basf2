#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import sys
import basf2 as b2

from ROOT import Belle2

if __name__ == "__main__":
    model_file = sys.argv[1]
    config_file = sys.argv[2]

    main = b2.create_path()

    eventinfosetter = b2.register_module('EventInfoSetter')
    eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
    main.add_module(eventinfosetter)

    iov = Belle2.IntervalOfValidity.always()

    db = Belle2.Database.Instance()
    db.addPayload('graFEIModelFile', model_file, iov)
    db.addPayload('graFEIConfigFile', config_file, iov)

    b2.process(main)
