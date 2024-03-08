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


class SaveModelToPayload(b2.Module):
    """
    Module to save model weight and yaml files to payload.
    """

    def initialize(self):
        """
        Called at the beginning
        """
        b2.B2INFO('Creating local database...')

    def beginRun(self):
        """
        Called at the beginning of each run.
        """
        from ROOT import Belle2

        iov = Belle2.IntervalOfValidity.always()

        db = Belle2.Database.Instance()
        db.addPayload('graFEIModelFile', model_file, iov)
        db.addPayload('graFEIConfigFile', config_file, iov)

    def terminate(self):
        """
        Called at the end.
        """
        b2.B2INFO('Done!')


if __name__ == "__main__":
    model_file = sys.argv[1]
    config_file = sys.argv[2]

    main = b2.create_path()

    eventinfosetter = b2.register_module('EventInfoSetter')
    eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
    main.add_module(eventinfosetter)

    modelSaver = b2.register_module(SaveModelToPayload())
    main.add_module(modelSaver)

    b2.process(main)
