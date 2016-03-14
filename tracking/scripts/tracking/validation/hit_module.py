#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math
import collections
import numpy as np

from tracking.validation.pull import PullAnalysis
from tracking.validation.fom import ValidationFiguresOfMerit, \
    ValidationManyFiguresOfMerit
from tracking.validation.module import AlwaysPassFilter, \
    getHelixFromMCParticle, TrackingValidationModule

import basf2

import ROOT
ROOT.gSystem.Load('libtracking')
from ROOT import Belle2


class ExpertTrackingValidationModule(TrackingValidationModule):

    """Module to collect more matching information about the found particles and to generate validation plots and figures of merit on the performance of track finding. This module gives information on the number of hits etc. """

    def __init__(
        self,
        name,
        contact,
        fit=False,
        pulls=False,
        output_file_name=None,
        track_filter_object=AlwaysPassFilter(),
        plot_name_postfix='',
        plot_title_postfix='',
        exclude_profile_mc_parameter='',
        exclude_profile_pr_parameter='',
        use_expert_folder=True,
        trackCandidatesColumnName='TrackCands',
        mcTrackCandidatesColumnName='MCTrackCands',
        cdcHitsColumnName='CDCHits',
        ):

        TrackingValidationModule.__init__(
            self,
            name,
            contact,
            fit,
            pulls,
            output_file_name,
            track_filter_object,
            plot_name_postfix,
            plot_title_postfix,
            exclude_profile_mc_parameter,
            exclude_profile_pr_parameter,
            use_expert_folder,
            trackCandidatesColumnName,
            mcTrackCandidatesColumnName,
            )

        self.cdcHitsColumnname = cdcHitsColumnName

    def initialize(self):
        TrackingValidationModule.initialize(self)
        # Use deques in favour of lists to prevent repeated memory allocation of cost O(n)

        self.number_of_total_hits = collections.deque()
        self.number_of_mc_hits = collections.deque()
        self.number_of_pr_hits = collections.deque()
        self.is_hit_found = collections.deque()
        self.is_hit_matched = collections.deque()

        # MC information
        self.mc_missing = collections.deque()
        self.ratio_hits_in_mc_tracks_and_not_in_pr_tracks = collections.deque()
        self.mc_is_primary = collections.deque()
        self.mc_number_of_hits = collections.deque()
        self.mc_number_of_missing_hits = collections.deque()  # the number of hits from the mc track cand that are not found by the pr track cand (in any track)

        # PT information
        self.number_of_connected_tracks = collections.deque()  # This is the number of mcTrackCands sharing a hit with the track cand.
        self.number_of_wrong_hits = collections.deque()  # This number gives information about the "badness" of the fake.
                                                         # It is calculated by going through all hits of the fake track and the connected mc track cands and counting the number.
                                                         # These numbers are than summed up and substracted by the biggest number of hits this candidates shares with the mc track cands.
        self.pr_number_of_hits = collections.deque()
        self.pr_number_of_matched_hits = collections.deque()

    def event(self):
        """Event method"""

        TrackingValidationModule.event(self)
        self.examine_hits_in_event()

    def examine_hits_in_event(self):

        trackCands = Belle2.PyStoreArray(self.trackCandidatesColumnName)
        mcTrackCands = Belle2.PyStoreArray(self.mcTrackCandidatesColumnName)
        cdcHits = Belle2.PyStoreArray(self.cdcHitsColumnname)

        totalHitListMC = set([cdcHitID for mcTrackCand in mcTrackCands
                             for cdcHitID in
                             mcTrackCand.getHitIDs(Belle2.Const.CDC)])
        totalHitListPR = set([cdcHitID for trackCand in trackCands
                             for cdcHitID in
                             trackCand.getHitIDs(Belle2.Const.CDC)])
        totalHitList = set([cdcHit.getArrayIndex() for cdcHit in cdcHits])

        number_of_mc_hits = len(totalHitListMC)
        number_of_pr_hits = len(totalHitListPR)
        number_of_all_hits = len(totalHitList)

        is_hit_matched = 0
        is_hit_found = len(totalHitListMC & totalHitListPR)

        for trackCand in trackCands:

            is_matched = self.trackMatchLookUp.isMatchedPRTrackCand(trackCand)
            is_clone = self.trackMatchLookUp.isClonePRTrackCand(trackCand)

            trackCandHits = set(trackCand.getHitIDs(Belle2.Const.CDC))

            # this is not very efficient...
            list_of_connected_mc_tracks = set()
            list_of_numbers_of_hits_for_connected_tracks = collections.deque()
            number_of_connected_tracks = 0
            number_of_wrong_hits = 0

            for mcTrackCand in mcTrackCands:
                mcTrackCandHits = set(mcTrackCand.getHitIDs(Belle2.Const.CDC))
                length_of_intersection = len(mcTrackCandHits & trackCandHits)
                if length_of_intersection > 0:
                    list_of_connected_mc_tracks.add(mcTrackCand)
                    list_of_numbers_of_hits_for_connected_tracks.append(length_of_intersection)

            if len(list_of_numbers_of_hits_for_connected_tracks) == 0:
                self.number_of_wrong_hits.append(0)
                self.pr_number_of_matched_hits.append(0)
            else:
                maximum_intersection = \
                    max(list_of_numbers_of_hits_for_connected_tracks)
                self.pr_number_of_matched_hits.append(sum(list_of_numbers_of_hits_for_connected_tracks))
                self.number_of_wrong_hits.append(sum(list_of_numbers_of_hits_for_connected_tracks)
                        - maximum_intersection)

            self.number_of_connected_tracks.append(len(list_of_connected_mc_tracks))

            if is_matched or is_clone:
                mcTrackCand = \
                    self.trackMatchLookUp.getRelatedMCTrackCand(trackCand)
                mcTrackCandHits = set(mcTrackCand.getHitIDs(Belle2.Const.CDC))

                is_hit_matched += len(trackCandHits & mcTrackCandHits)

            self.pr_number_of_hits.append(len(trackCandHits))

        for mcTrackCand in mcTrackCands:
            is_missing = \
                self.trackMatchLookUp.isMissingMCTrackCand(mcTrackCand)

            mcTrackCandHits = set(mcTrackCand.getHitIDs(Belle2.Const.CDC))

            ratio = 1.0 - 1.0 * len(mcTrackCandHits & totalHitListPR) \
                / len(mcTrackCandHits)
            self.ratio_hits_in_mc_tracks_and_not_in_pr_tracks.append(ratio)

            mcParticle = \
                self.trackMatchLookUp.getRelatedMCParticle(mcTrackCand)
            is_primary = \
                mcParticle.hasStatus(Belle2.MCParticle.c_PrimaryParticle)
            self.mc_is_primary.append(is_primary)
            self.mc_number_of_hits.append(len(mcTrackCandHits))

            self.mc_number_of_missing_hits.append(len(mcTrackCandHits)
                    - len(mcTrackCandHits & totalHitListPR))

            self.mc_missing.append(is_missing)

        self.number_of_total_hits.append(number_of_all_hits)
        self.number_of_mc_hits.append(number_of_mc_hits)
        self.number_of_pr_hits.append(number_of_pr_hits)

        self.is_hit_found.append(is_hit_found)
        self.is_hit_matched.append(is_hit_matched)

    def terminate(self):
        TrackingValidationModule.terminate(self)

        output_tfile = ROOT.TFile(self.output_file_name, 'update')

        # MC Figures of merit
        mc_figures_of_merit = \
            ValidationManyFiguresOfMerit('%s_mc_figures_of_merit' % self.name)

        mc_figures_of_merit['mc_pts'] = self.mc_pts
        mc_figures_of_merit['mc_d0s'] = self.mc_d0s
        mc_figures_of_merit['mc_matches'] = self.mc_matches
        mc_figures_of_merit['mc_hit_efficiencies'] = self.mc_hit_efficiencies
        mc_figures_of_merit['mc_multiplicities'] = self.mc_multiplicities
        mc_figures_of_merit['mc_phis'] = self.mc_phi
        mc_figures_of_merit['mc_tan_lambdas'] = self.mc_tan_lambdas
        mc_figures_of_merit['mc_thetas'] = self.mc_theta
        mc_figures_of_merit['mc_missing'] = self.mc_missing
        mc_figures_of_merit['mc_is_primary'] = self.mc_is_primary
        mc_figures_of_merit['mc_number_of_hits'] = self.mc_number_of_hits
        mc_figures_of_merit['mc_number_of_missing_hits'] = \
            self.mc_number_of_missing_hits
        mc_figures_of_merit['ratio_hits_in_mc_tracks_and_not_in_pr_tracks'] = \
            self.ratio_hits_in_mc_tracks_and_not_in_pr_tracks

        mc_figures_of_merit.write()

        # PR Figures of merit
        pr_figures_of_merit = \
            ValidationManyFiguresOfMerit('%s_pr_figures_of_merit' % self.name)

        pr_figures_of_merit['pr_clones_and_matches'] = \
            self.pr_clones_and_matches
        pr_figures_of_merit['pr_matches'] = self.pr_matches
        pr_figures_of_merit['pr_fakes'] = self.pr_fakes
        pr_figures_of_merit['pr_number_of_hits'] = self.pr_number_of_hits
        pr_figures_of_merit['pr_number_of_matched_hits'] = \
            self.pr_number_of_matched_hits
        pr_figures_of_merit['pr_seed_tan_lambdas'] = self.pr_seed_tan_lambdas
        pr_figures_of_merit['pr_seed_phi'] = self.pr_seed_phi
        pr_figures_of_merit['pr_seed_theta'] = self.pr_seed_theta

        pr_figures_of_merit['number_of_connected_tracks'] = \
            self.number_of_connected_tracks
        pr_figures_of_merit['number_of_wrong_hits'] = self.number_of_wrong_hits

        pr_figures_of_merit.write()

        # Hit Figures of merit
        hit_figures_of_merit = \
            ValidationFiguresOfMerit('%s_hit_figures_of_merit' % self.name)

        hit_figures_of_merit['number_of_total_hits'] = \
            np.sum(self.number_of_total_hits)
        hit_figures_of_merit['number_of_mc_hits'] = \
            np.sum(self.number_of_mc_hits)
        hit_figures_of_merit['number_of_pr_hits'] = \
            np.sum(self.number_of_pr_hits)
        hit_figures_of_merit['is_hit_found'] = np.sum(self.is_hit_found)
        hit_figures_of_merit['is_hit_matched'] = np.sum(self.is_hit_matched)

        print hit_figures_of_merit
        hit_figures_of_merit.write()

        output_tfile.Close()


