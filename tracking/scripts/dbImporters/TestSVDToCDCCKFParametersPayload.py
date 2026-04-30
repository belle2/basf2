#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Run with: basf2 TestSVDToCDCCKFParametersPayload.py
"""

import ROOT
from ROOT import Belle2


PAYLOAD_NAME = "SVDToCDCCKFParameters"
LOCAL_DB_FILE = "localdb/database.txt"


def default_payload(p):
    """Create and return a default SVDToCDCCKFParameters payload.
    Values match those in SVDToCDCCKFParameters.h constructor."""

    # Float parameters
    p.setMaximalDeltaPhi(ROOT.TMath.Pi() / 8)
    p.setMinimalPtRequirement(0.0)

    # Integer parameters
    p.setMaximalLayerJump(2)
    p.setMaximalLayerJumpBackwardSeed(3)
    p.setPathMaximalCandidatesInFlight(3)
    p.setStateMaximalHitCandidates(4)

    return p


def print_payload(p):
    """Print the payload values."""

    print(f"\n{PAYLOAD_NAME} payload values:")
    print()
    print(f"  maximalDeltaPhi                = {p.getMaximalDeltaPhi()}")
    print(f"  minimalPtRequirement           = {p.getMinimalPtRequirement()}")
    print(f"  maximalLayerJump               = {p.getMaximalLayerJump()}")
    print(f"  maximalLayerJumpBackwardSeed   = {p.getMaximalLayerJumpBackwardSeed()}")
    print(f"  pathMaximalCandidatesInFlight  = {p.getPathMaximalCandidatesInFlight()}")
    print(f"  stateMaximalHitCandidates      = {p.getStateMaximalHitCandidates()}")
    print()


def main():

    # Create a test/default payload
    p = Belle2.SVDToCDCCKFParameters()
    p = default_payload(p)

    # Print the payload
    print_payload(p)

    # Store the payload to local conditions DB
    iov = Belle2.IntervalOfValidity(0, 0, -1, -1)
    Belle2.Database.Instance().storeData(PAYLOAD_NAME, p, iov)

    # Print storage info
    print(f"\n{PAYLOAD_NAME} payload stored.")
    print(f"Metadata : {LOCAL_DB_FILE}")
    print("Files    : localdb/")


if __name__ == "__main__":
    main()
