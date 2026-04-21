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

    # String parameters (32 bytes)
    p.setFilter("size")
    p.setHitFindingDirection("forward")
    p.setInputRecoTrackStoreArrayName("SVDPlusCDCStandaloneRecoTracks")
    p.setInputWireHits("CDCWireHitVector")
    p.setOutputRecoTrackStoreArrayName("CKFCDCRecoTracks")
    p.setOutputRelationRecoTrackStoreArrayName("SVDPlusCDCStandaloneRecoTracks")
    p.setPathFilter("arc_length")
    p.setRelatedRecoTrackStoreArrayName("CKFCDCRecoTracks")
    p.setRelationCheckForDirection("backward")
    p.setSeedComponent("SVD")
    p.setStateBasicFilter("rough")
    p.setStateExtrapolationFilter("extrapolate_and_update")
    p.setStateFinalFilter("distance")
    p.setStatePreFilter("all")
    p.setTrackFindingDirection("forward")
    p.setWriteOutDirection("backward")

    # Float parameters (4 bytes)
    p.setMaximalDeltaPhi(ROOT.TMath.Pi() / 8)
    p.setMinimalPtRequirement(0.0)

    # Integer parameters (4 bytes)
    p.setMaximalLayerJump(2)
    p.setMaximalLayerJumpBackwardSeed(3)
    p.setPathMaximalCandidatesInFlight(3)
    p.setStateMaximalHitCandidates(4)

    # Boolean parameters (1 byte)
    p.setExportTracks(True)
    p.setExportAllTracks(False)
    p.setIgnoreTracksWithCDChits(False)
    p.setTakenFlag(True)

    # Filter parameter maps - stored as map<string, double> for ROOT compatibility
    # All numeric values are stored as double; findlet casts to appropriate type

    p.setFilterParameters({})
    p.setPathFilterParameters({})
    p.setStateBasicFilterParameters({"maximalHitDistance": 0.15})
    p.setStateExtrapolationFilterParameters({})
    p.setStateFinalFilterParameters({})
    p.setStatePreFilterParameters({})

    return p


def test_payload():
    """Function to create a test SVDToCDCCKFParameters payload."""
    p = Belle2.SVDToCDCCKFParameters()

    # String parameters (32 bytes)
    p.setFilter("size")
    p.setHitFindingDirection("forward")
    p.setInputRecoTrackStoreArrayName("SVDPlusCDCStandaloneRecoTracks")
    p.setInputWireHits("CDCWireHitVector")
    p.setOutputRecoTrackStoreArrayName("RecoTracks")
    p.setOutputRelationRecoTrackStoreArrayName("SVDPlusCDCStandaloneRecoTracks")
    p.setPathFilter("distance")
    p.setRelatedRecoTrackStoreArrayName("RecoTracks")
    p.setRelationCheckForDirection("backward")
    p.setSeedComponent("SVD")
    p.setStateBasicFilter("rough")
    p.setStateExtrapolationFilter("extrapolate_and_update")
    p.setStateFinalFilter("distance")
    p.setStatePreFilter("all")
    p.setTrackFindingDirection("forward")
    p.setWriteOutDirection("backward")

    # Float parameters (4 bytes)
    p.setMaximalDeltaPhi(0.4)
    p.setMinimalPtRequirement(0.50)

    # Integer parameters (4 bytes)
    p.setMaximalLayerJump(6)
    p.setMaximalLayerJumpBackwardSeed(3)
    p.setPathMaximalCandidatesInFlight(3)
    p.setStateMaximalHitCandidates(4)

    # Boolean parameters (1 byte)
    p.setExportTracks(True)
    p.setExportAllTracks(False)
    p.setIgnoreTracksWithCDChits(False)
    p.setTakenFlag(True)

    # Filter parameter maps - using unified map<string, double> API
    p.setFilterParameters({"testParam1": 15})
    p.setPathFilterParameters({"testParam2": True})
    p.setStateBasicFilterParameters({"maximalHitDistance": 0.25})
    p.setStateExtrapolationFilterParameters({"testParam3": "test_string"})
    p.setStateFinalFilterParameters({"testParam4": "string_list"})
    p.setStatePreFilterParameters({"testParam5": 1.5})

    return p


def print_payload(tag, p):
    print(f"\n{'='*60}\n  {tag}\n{'='*60}")
    # String parameters (32 bytes)
    print(f"  filter                         = '{p.getFilter()}'")
    print(f"  hitFindingDirection            = '{p.getHitFindingDirection()}'")
    print(f"  inputRecoTrackStoreArrayName   = '{p.getInputRecoTrackStoreArrayName()}'")
    print(f"  inputWireHits                  = '{p.getInputWireHits()}'")
    print(f"  outputRecoTrackStoreArrayName  = '{p.getOutputRecoTrackStoreArrayName()}'")
    print(f"  outputRelationRecoTrackStoreArrayName = '{p.getOutputRelationRecoTrackStoreArrayName()}'")
    print(f"  pathFilter                     = '{p.getPathFilter()}'")
    print(f"  relatedRecoTrackStoreArrayName = '{p.getRelatedRecoTrackStoreArrayName()}'")
    print(f"  relationCheckForDirection      = '{p.getRelationCheckForDirection()}'")
    print(f"  seedComponent                  = '{p.getSeedComponent()}'")
    print(f"  stateBasicFilter               = '{p.getStateBasicFilter()}'")
    print(f"  stateExtrapolationFilter       = '{p.getStateExtrapolationFilter()}'")
    print(f"  stateFinalFilter               = '{p.getStateFinalFilter()}'")
    print(f"  statePreFilter                 = '{p.getStatePreFilter()}'")
    print(f"  trackFindingDirection          = '{p.getTrackFindingDirection()}'")
    print(f"  writeOutDirection              = '{p.getWriteOutDirection()}'")
    # Float parameters (4 bytes)
    print(f"  maximalDeltaPhi                = {p.getMaximalDeltaPhi()}")
    print(f"  minimalPtRequirement           = {p.getMinimalPtRequirement()}")
    # Integer parameters (4 bytes)
    print(f"  maximalLayerJump               = {p.getMaximalLayerJump()}")
    print(f"  maximalLayerJumpBackwardSeed   = {p.getMaximalLayerJumpBackwardSeed()}")
    print(f"  pathMaximalCandidatesInFlight  = {p.getPathMaximalCandidatesInFlight()}")
    print(f"  stateMaximalHitCandidates      = {p.getStateMaximalHitCandidates()}")
    # Boolean parameters (1 byte)
    print(f"  exportTracks                   = {p.getExportTracks()}")
    print(f"  exportAllTracks                = {p.getExportAllTracks()}")
    print(f"  ignoreTracksWithCDChits        = {p.getIgnoreTracksWithCDChits()}")
    print(f"  takenFlag                      = {p.getTakenFlag()}")
    # Filter parameter maps (unified map<string, double>)
    print(f"  filterParameters         size  = {p.getFilterParameters().size()}")
    print(f"  pathFilterParameters     size  = {p.getPathFilterParameters().size()}")
    print(f"  stateBasicFilterParams   size  = {p.getStateBasicFilterParameters().size()}")
    print(f"  stateExtrapolationFilterParams size = {p.getStateExtrapolationFilterParameters().size()}")
    print(f"  stateFinalFilterParams   size  = {p.getStateFinalFilterParameters().size()}")
    print(f"  statePreFilterParams     size  = {p.getStatePreFilterParameters().size()}")


def main():

    # Store directly to local conditions DB using DBImportObjPtr —
    # no intermediate ROOT file needed.
    print("\n" + "="*60)
    print("  Store payload in local conditions DB")
    print("="*60)

    p = test_payload()
    print_payload("payload to be stored", p)

    iov = Belle2.IntervalOfValidity(0, 0, -1, -1)
    Belle2.Database.Instance().storeData(PAYLOAD_NAME, p, iov)

    try:
        from ROOT import Belle2 as _Belle2
        ptr = _Belle2.DBObjPtr(_Belle2.SVDToCDCCKFParameters)(PAYLOAD_NAME)
        if ptr:
            print_payload("payload reloaded", ptr.get())
        else:
            print("[FilterParamMap diagnostic] Could not reload payload via DBObjPtr")
    except Exception as e:
        print("[FilterParamMap diagnostic] Reload test failed:", repr(e))

    print(f"\n  Payload '{PAYLOAD_NAME}' stored.")
    print(f"  Metadata : {LOCAL_DB_FILE}")
    print("  Files    : localdb/")


if __name__ == "__main__":
    main()
