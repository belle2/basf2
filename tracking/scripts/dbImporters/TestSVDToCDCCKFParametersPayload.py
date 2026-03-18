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


def make_payload():
    """Create and return a fully populated SVDToCDCCKFParameters object."""
    p = Belle2.SVDToCDCCKFParameters()

    p.setMaximalDeltaPhi(0.5)
    p.setMinimalPtRequirement(0.25)
    p.setMaximalLayerJump(4)
    p.setMaximalLayerJumpBackwardSeed(6)
    p.setPathMaximalCandidatesInFlight(2)
    p.setStateMaximalHitCandidates(3)

    p.setExportTracks(True)
    p.setExportAllTracks(False)
    p.setIgnoreTracksWithCDChits(True)
    p.setTakenFlag(True)

    p.setFilter("recording")
    p.setHitFindingDirection("forward")
    p.setInputRecoTrackStoreArrayName("RecoTracks")
    p.setInputWireHits("CDCWireHitVector")
    p.setOutputRecoTrackStoreArrayName("CKFRecoTracks")
    p.setOutputRelationRecoTrackStoreArrayName("RecoTracks")
    p.setPathFilter("recording")
    p.setRelatedRecoTrackStoreArrayName("RecoTracks")
    p.setRelationCheckForDirection("invalid")
    p.setSeedComponent("SVD")
    p.setStateBasicFilter("rough")
    p.setStateExtrapolationFilter("extrapolate_and_update")
    p.setStateFinalFilter("distance")
    p.setStatePreFilter("all")
    p.setTrackFindingDirection("forward")
    p.setWriteOutDirection("both")

    p.setFilterParametersFloat({"cutValue": 0.75})
    p.setFilterParametersInt({"maxHits": 10})
    p.setFilterParametersBool({"useFilter": True})
    p.setFilterParametersStr({"name": "myFilter"})
    p.setFilterParametersVecStr({"components": ["SVD", "CDC"]})

    p.setStateBasicFilterParametersFloat({"maximalHitDistance": 0.15})
    p.setStateBasicFilterParametersInt({"maxLayer": 6})
    p.setStateExtrapolationFilterParametersFloat({"maximalWeight": 0.9})
    p.setStateFinalFilterParametersFloat({"cutValue": 0.5})
    p.setStateFinalFilterParametersBool({"invert": False})
    p.setPathFilterParametersFloat({"cutValue": 0.6})
    p.setStatePreFilterParametersStr({"direction": "forward"})

    return p


def print_payload(tag, p):
    print(f"\n{'='*60}\n  {tag}\n{'='*60}")
    print(f"  maximalDeltaPhi                = {p.getMaximalDeltaPhi()}")
    print(f"  minimalPtRequirement           = {p.getMinimalPtRequirement()}")
    print(f"  maximalLayerJump               = {p.getMaximalLayerJump()}")
    print(f"  maximalLayerJumpBackwardSeed   = {p.getMaximalLayerJumpBackwardSeed()}")
    print(f"  pathMaximalCandidatesInFlight  = {p.getPathMaximalCandidatesInFlight()}")
    print(f"  stateMaximalHitCandidates      = {p.getStateMaximalHitCandidates()}")
    print(f"  exportTracks                   = {p.getExportTracks()}")
    print(f"  exportAllTracks                = {p.getExportAllTracks()}")
    print(f"  ignoreTracksWithCDChits        = {p.getIgnoreTracksWithCDChits()}")
    print(f"  takenFlag                      = {p.getTakenFlag()}")
    print(f"  filter                         = '{p.getFilter()}'")
    print(f"  hitFindingDirection            = '{p.getHitFindingDirection()}'")
    print(f"  inputRecoTrackStoreArrayName   = '{p.getInputRecoTrackStoreArrayName()}'")
    print(f"  outputRecoTrackStoreArrayName  = '{p.getOutputRecoTrackStoreArrayName()}'")
    print(f"  seedComponent                  = '{p.getSeedComponent()}'")
    print(f"  stateBasicFilter               = '{p.getStateBasicFilter()}'")
    print(f"  stateFinalFilter               = '{p.getStateFinalFilter()}'")
    print(f"  trackFindingDirection          = '{p.getTrackFindingDirection()}'")
    print(f"  writeOutDirection              = '{p.getWriteOutDirection()}'")
    print(f"  filterParametersFloat  size    = {p.getFilterParametersFloat().size()}")
    print(f"  filterParametersInt    size    = {p.getFilterParametersInt().size()}")
    print(f"  filterParametersBool   size    = {p.getFilterParametersBool().size()}")
    print(f"  filterParametersStr    size    = {p.getFilterParametersStr().size()}")
    print(f"  filterParametersVecStr size    = {p.getFilterParametersVecStr().size()}")
    print(f"  stateBasicFilterParamsFloat    = {p.getStateBasicFilterParametersFloat().size()}")
    print(f"  stateFinalFilterParamsFloat    = {p.getStateFinalFilterParametersFloat().size()}")


def main():

    # Store directly to local conditions DB using DBImportObjPtr —
    # no intermediate ROOT file needed.
    print("\n" + "="*60)
    print("  Store payload in local conditions DB")
    print("="*60)

    p = make_payload()
    print_payload("payload to be stored", p)

    iov = Belle2.IntervalOfValidity(0, 0, -1, -1)
    Belle2.Database.Instance().storeData(PAYLOAD_NAME, p, iov)

    print(f"\n  Payload '{PAYLOAD_NAME}' stored.")
    print(f"  Metadata : {LOCAL_DB_FILE}")
    print("  Files    : localdb/")


if __name__ == "__main__":
    main()
