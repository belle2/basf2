#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""This file contains some functions used in the validation package to
generate the Monte Carlo events in a reproducible manner."""


from basf2 import find_file
from pdg import load


def add_evtgen_for_validation(path):
    """
    Add the EvtGenInput module with the correct decfile and pdl files to be used for the nightly validation.
    """
    # Load the appropriate pdl file
    load(find_file("data/validation/validation.pdl"))
    # And add EvtGen with the appropriate decfile
    path.add_module(
        "EvtGenInput",
        DECFile=find_file("data/validation/validation.dec")
    )
