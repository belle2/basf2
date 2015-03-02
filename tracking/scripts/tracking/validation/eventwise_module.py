#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math
import numpy as np

import tracking.validation.harvesting as harvesting
import tracking.validation.refiners as refiners

import basf2

import ROOT
ROOT.gSystem.Load("libtracking")
from ROOT import Belle2


class EventwiseTrackingValidationModule(harvesting.HarvestingModule):

    def __init__(self,
                 name,
                 contact,
                 output_file_name=None,
                 trackCandidatesColumnName='TrackCands',
                 expert_level=None):

        output_file_name = output_file_name or name + 'TrackingValidation.root'

        super(EventwiseTrackingValidationModule, self).__init__(foreach="EventMetaData",  # Dummy for on element per event
                                                                name=name,
                                                                output_file_name=output_file_name,
                                                                contact=contact,
                                                                expert_level=expert_level)

        self.trackCandidatesColumnName = trackCandidatesColumnName
        self.mcTrackCandidatesColumnName = "MCTrackCands"
        self.cdcHitsColumnname = "CDCHits"

    def initialize(self):
        super(EventwiseTrackingValidationModule, self).initialize()
        self.trackMatchLookUp = Belle2.TrackMatchLookUp(self.mcTrackCandidatesColumnName, self.trackCandidatesColumnName)

    def pick(self, event_meta_data=None):
        return True

    def peel(self, event_meta_data=None):
        # Note event_meta_data is just used as a dummy.
        trackMatchLookUp = self.trackMatchLookUp

        trackCands = Belle2.PyStoreArray(self.trackCandidatesColumnName)
        mcTrackCands = Belle2.PyStoreArray(self.mcTrackCandidatesColumnName)
        cdcHits = Belle2.PyStoreArray(self.cdcHitsColumnname)

        totalHitListMC = set()
        for mcTrackCand in mcTrackCands:
            totalHitListMC.update(mcTrackCand.getHitIDs(Belle2.Const.CDC))

        totalHitListPR = set()
        is_hit_matched = 0

        for trackCand in trackCands:
            is_matched = self.trackMatchLookUp.isMatchedPRTrackCand(trackCand)
            is_clone = self.trackMatchLookUp.isClonePRTrackCand(trackCand)

            trackCandHits = set(trackCand.getHitIDs(Belle2.Const.CDC))
            totalHitListPR.update(trackCandHits)

            if is_matched or is_clone:
                mcTrackCand = self.trackMatchLookUp.getRelatedMCTrackCand(trackCand)
                mcTrackCandHits = set(mcTrackCand.getHitIDs(Belle2.Const.CDC))

                is_hit_matched += len(trackCandHits & mcTrackCandHits)

        return dict(
            number_of_total_hits=cdcHits.getEntries(),
            number_of_mc_hits=len(totalHitListMC),
            number_of_pr_hits=len(totalHitListPR),
            is_hit_found=len(totalHitListMC & totalHitListPR),
            is_hit_matched=is_hit_matched
        )

    # Refiners to be executed on terminate #
    # #################################### #

    # Save a tree of all collected variables in a sub folder
    save_tree = refiners.save_tree(folder_name="event_tree", name="event_tree", above_expert_level=1)

    save_clone_rate = refiners.save_fom(
        name="{module.name}_hit_figures_of_merit",
        title="Hit sums in {module.title}",
        description="",  # to be given
        select=["number_of_total_hits",
                "number_of_mc_hits",
                "number_of_pr_hits",
                "is_hit_found",
                "is_hit_matched",
                ],
        aggregation=np.sum,
        key="{part_name}",
    )


def main():
    eventwiseTrackingValidationModule = EventwiseTrackingValidationModule(name='event_test', contact='dummy')
    eventwiseTrackingValidationModule.run('tracked_gun100.root')


if __name__ == '__main__':
    import logging
    logging.basicConfig(level=logging.DEBUG)
    main()
