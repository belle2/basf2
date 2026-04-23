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

from ROOT import Belle2


PAYLOAD_NAME = "SVDToCDCCKFParameters"
LOCAL_DB_FILE = "localdb/database.txt"


def default_payload():
    """Function to create a default SVDToCDCCKFParameters payload.
    Default values match those in SVDToCDCCKFParameters.h constructor.
    """
    import ROOT
    p = Belle2.SVDToCDCCKFParameters()

    p.setMaximalDeltaPhi(ROOT.TMath.Pi() / 8)
    p.setMinimalPtRequirement(0.0)
    p.setMaximalLayerJump(2)
    p.setMaximalLayerJumpBackwardSeed(3)
    p.setPathMaximalCandidatesInFlight(3)
    p.setStateMaximalHitCandidates(4)

    return p


def test_payload():
    """Function to create a test SVDToCDCCKFParameters payload."""

    p = Belle2.SVDToCDCCKFParameters()

    p.setMaximalDeltaPhi(0.5)
    p.setMinimalPtRequirement(0.5)
    p.setMaximalLayerJump(9)
    p.setMaximalLayerJumpBackwardSeed(9)
    p.setPathMaximalCandidatesInFlight(9)
    p.setStateMaximalHitCandidates(9)

    return p


def print_payload(tag, p):
    print(f"\n{'='*50}\n  Start of {tag}\n{'='*50}")
    print(f"  maximalDeltaPhi                = {p.getMaximalDeltaPhi()}")
    print(f"  minimalPtRequirement           = {p.getMinimalPtRequirement()}")
    print(f"  maximalLayerJump               = {p.getMaximalLayerJump()}")
    print(f"  maximalLayerJumpBackwardSeed   = {p.getMaximalLayerJumpBackwardSeed()}")
    print(f"  pathMaximalCandidatesInFlight  = {p.getPathMaximalCandidatesInFlight()}")
    print(f"  stateMaximalHitCandidates      = {p.getStateMaximalHitCandidates()}")

    print(f"{'='*50}\n  End of {tag}\n{'='*50}\n")


def main():

    p = default_payload()
    print_payload("payload to be stored", p)

    iov = Belle2.IntervalOfValidity(0, 0, -1, -1)
    Belle2.Database.Instance().storeData(PAYLOAD_NAME, p, iov)

    print(f"\n  Payload '{PAYLOAD_NAME}' stored.")
    print(f"  Metadata : {LOCAL_DB_FILE}")
    print("  Files    : localdb/")


if __name__ == "__main__":
    main()
