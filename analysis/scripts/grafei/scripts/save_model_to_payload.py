#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import sys
import torch
import tempfile
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
    db.addPayload('graFEIConfigFile', config_file, iov)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = torch.load(model_file, map_location=torch.device(device))

    with tempfile.NamedTemporaryFile(suffix=".pt", delete=False) as temp_file:
        temp_model_path = temp_file.name
        torch.save({"model": model["model"]}, temp_model_path)
        db.addPayload('graFEIModelFile', temp_model_path, iov)

    b2.process(main)
