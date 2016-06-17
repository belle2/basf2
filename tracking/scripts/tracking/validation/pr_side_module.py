import basf2

import ROOT
ROOT.gSystem.Load("libtracking")
from ROOT import Belle2

import math
import warnings

import numpy as np

import tracking.validation.harvesting as harvesting
import tracking.validation.refiners as refiners
import tracking.validation.peelers as peelers
import tracking.validation.utilities as utilities


class PRSideTrackingValidationModule(harvesting.HarvestingModule):
    """Module to collect matching information about the found particles and to generate
       validation plots and figures of merit on the performance of track finding."""

    def __init__(self,
                 name,
                 contact,
                 output_file_name=None,
                 track_cands_name="TrackCands",
                 mc_track_cands_name='MCTrackCands',
                 expert_level=None):

        output_file_name = output_file_name or name + 'TrackingValidation.root'
        super().__init__(foreach=track_cands_name,
                         name=name,
                         contact=contact,
                         output_file_name=output_file_name,
                         expert_level=expert_level)

        #: Name of the StoreArray of the tracks from pattern recognition
        self.track_cands_name = track_cands_name

        #: Name of the StoreArray of the ideal mc tracks
        self.mc_track_cands_name = mc_track_cands_name

        #: Reference to the track match lookup object reading the relation information constructed by the MCMatcherTracksModule
        self.track_match_look_up = None

        #: Cache for the hit content of the Monte Carlo tracks - updated each event
        self.mc_track_cands_det_hit_ids = []

    def initialize(self):
        """Receive signal at the start of event processing"""
        super().initialize()
        self.track_match_look_up = Belle2.TrackMatchLookUp(self.mc_track_cands_name,
                                                           self.track_cands_name)

    def prepare(self):
        """Called once at the start of each event"""
        super().prepare()
        mc_track_cands = Belle2.PyStoreArray(self.mc_track_cands_name)
        mc_track_cands_det_hit_ids = []

        for mc_track_cand in mc_track_cands:
            mc_track_cand_det_hit_ids = utilities.get_det_hit_ids(mc_track_cand)
            mc_track_cands_det_hit_ids.append(mc_track_cand_det_hit_ids)

        self.mc_track_cands_det_hit_ids = mc_track_cands_det_hit_ids

    def pick(self, track_cand):
        """Method to filter the track candidates to reject part of them"""
        return True

    def peel(self, track_cand):
        """Looks at the individual pattern recognition tracks and store information about them"""
        track_match_look_up = self.track_match_look_up

        # Matching information
        mc_particle = track_match_look_up.getRelatedMCParticle(track_cand)
        mc_particle_crops = peelers.peel_mc_particle(mc_particle)

        hit_content_crops = peelers.peel_track_cand_hit_content(track_cand)

        pr_to_mc_match_info_crops = self.peel_pr_to_mc_match_info(track_cand)

        # Get the fit results
        seed_fit_crops = peelers.peel_track_cand_seed(track_cand)

        fit_result = track_match_look_up.getRelatedTrackFitResult(track_cand)
        fit_crops = peelers.peel_track_fit_result(fit_result)

        return dict(
            **mc_particle_crops,
            **hit_content_crops,
            **pr_to_mc_match_info_crops,
            **seed_fit_crops,
            **fit_crops,
        )

    def peel_pr_to_mc_match_info(self, track_cand):
        track_match_look_up = self.track_match_look_up
        is_matched = track_match_look_up.isMatchedPRTrackCand(track_cand)
        is_clone = track_match_look_up.isClonePRTrackCand(track_cand)
        is_background = track_match_look_up.isBackgroundPRTrackCand(track_cand)
        is_ghost = track_match_look_up.isGhostPRTrackCand(track_cand)

        track_cand_det_hit_ids = utilities.get_det_hit_ids(track_cand)
        n_intersecting_mc_tracks = 0
        for mc_track_cand_det_hit_ids in self.mc_track_cands_det_hit_ids:
            intersects = len(mc_track_cand_det_hit_ids & track_cand_det_hit_ids) > 0
            if intersects:
                n_intersecting_mc_tracks += 1

        return dict(
            is_matched=is_matched,
            is_clone=is_clone,
            is_background=is_background,
            is_ghost=is_ghost,
            is_clone_or_match=(is_matched or is_clone),
            is_fake=not (is_matched or is_clone),
            hit_efficiency=track_match_look_up.getRelatedEfficiency(track_cand),
            hit_purity=track_match_look_up.getRelatedPurity(track_cand),
            n_intersecting_mc_tracks=n_intersecting_mc_tracks,
        )

    # Refiners to be executed on terminate #
    # #################################### #

    # Save a tree of all collected variables in a sub folder
    save_tree = refiners.save_tree(folder_name="pr_tree", name="pr_tree", above_expert_level=1)

    save_clone_rate = refiners.save_fom(
        name="{module.id}_overview_figures_of_merit",
        # Same as in the mc side module to combine the overview figures of merit into the same TNTuple
        title="Overview figures in {module.title}",
        description="clone_rate - ratio of clones divided the number of tracks that are related to a particle (clones and matches)",
        key="clone rate",
        select=["is_clone"],
        aggregation=np.mean,
        filter_on="is_clone_or_match",
    )

    save_fake_rate = refiners.save_fom(
        name="{module.id}_overview_figures_of_merit",
        # Same as in the mc side module to combine the overview figures of merit into the same TNTuple
        title="Overview figures in {module.title}",
        description="fake_rate - ratio of pattern recognition tracks that are not related to a particle" +
                    "(background, ghost) to all pattern recognition tracks",
        key="fake rate",
        select="is_fake",
        aggregation=np.mean,
    )

    # Make profiles of the finding efficiencies versus various fit parameters
    # Rename the quatities to names that display nicely by root latex translation
    save_fake_rate_by_seed_phi0_profile = refiners.save_profiles(
        select={
            'is_fake': 'fake rate',
            'seed_phi0_estimate': 'seed #phi',
        },
        y='fake rate',
        y_binary=True,
        outlier_z_score=5.0,
    )

    save_fake_rate_by_seed_tan_lambda_profile = refiners.save_profiles(
        select={
            'is_fake': 'fake rate',
            'seed_tan_lambda_estimate': 'seed tan #lambda',
        },
        y='fake rate',
        y_binary=True,
        outlier_z_score=5.0,
        lower_bound=-1.73,
        upper_bound=3.27,
    )

    # Hit counts in each sub detector by the true pt value
    save_hit_counts_by_pt_profile = refiners.save_profiles(
        filter_on="is_matched",
        select={
            "pt_truth": "true p_{t}",
            "n_pxd_hits": "pxd hits",
            "n_svd_hits": "svd hits",
            "n_cdc_hits": "cdc hits",
        },
        y=[
            "pxd hits",
            "svd hits",
            "cdc hits",
        ]
    )

    #: Creates a distribution of p values from the Genfit track fit for match pr tracks.
    save_p_value_histogram = refiners.save_histograms(
        filter_on="is_matched",
        select={"p_value": "Genfit p value"},
        description="""
The distribution of p values from the Genfit track fit.
If all errors are propagated correctly the distribution should be flat.
Generally some peaking behvaiour towards zero is too be expected if the errors are underestimated.
""",
        check="The distribution should be flat."
    )

    # Pulls of seed parameters
    save_seed_omega_pull_analysis = refiners.save_pull_analysis(
        filter_on="is_matched",
        part_name="seed_omega",
        quantity_name="seed #omega",
        folder_name="pull_seed_omega",
        truth_name="omega_truth",
        unit="1/cm",
    )

    save_seed_tan_lambda_pull_analysis = refiners.save_pull_analysis(
        filter_on="is_matched",
        part_name="seed_tan_lambda",
        quantity_name="seed tan #lambda",
        folder_name="pull_seed_tan_lambda",
        truth_name="tan_lambda_truth",
    )

    # Pull of fitted parameters
    save_fitted_omega_pull_analysis = refiners.save_pull_analysis(
        filter_on="is_matched",
        part_name="omega",
        quantity_name="#omega",
        folder_name="pull_fitted_omega",
        unit="1/cm",
    )

    save_fitted_tan_lambda_pull_analysis = refiners.save_pull_analysis(
        filter_on="is_matched",
        part_name="tan_lambda",
        quantity_name="tan #lambda",
        folder_name="pull_fitted_tan_lambda",
    )

    save_fitted_pt_pull_analysis = refiners.save_pull_analysis(
        filter_on="is_matched",
        part_name="pt",
        quantity_name="p_{t}",
        folder_name="pull_fitted_p_t",
    )

    save_fitted_x_pull_analysis = refiners.save_pull_analysis(
        filter_on="is_matched",
        part_name="x",
        quantity_name="x",
        folder_name="pull_fitted_x{groupby_addition}",
        groupby=[None, ("pt_truth", [0.070, 0.250, 0.600])],
    )

    save_fitted_y_pull_analysis = refiners.save_pull_analysis(
        filter_on="is_matched",
        part_name="y",
        quantity_name="y",
        folder_name="pull_fitted_y{groupby_addition}",
        groupby=[None, ("pt_truth", [0.070, 0.250, 0.600])],
    )

    save_fitted_z_pull_analysis = refiners.save_pull_analysis(
        filter_on="is_matched",
        part_name="z",
        quantity_name="z",
        folder_name="pull_fitted_z{groupby_addition}",
        groupby=[None, ("pt_truth", [0.070, 0.250, 0.600])],
    )

    # Resolutions as a function of true p_t
    save_resolutions_by_pt_profile = refiners.save_profiles(
        filter_on="is_matched",
        select={
            "pt_truth": "true p_{t}",
            "d0_variance": "#sigma(d_{0})",
            "z0_variance": "#sigma(z_{0})",
            "pt_resolution": "#sigma(p_{t}) / p_{t}",
        },
        y=[
            "#sigma(d_{0})",
            "#sigma(z_{0})",
            "#sigma(p_{t}) / p_{t}",
        ],
        y_log=True,
    )


class ExpertPRSideTrackingValidationModule(PRSideTrackingValidationModule):
    """Module to collect more matching information about the found particles and to
       generate validation plots and figures of merit on the performance of track finding.
       This module gives information on the number of hits etc."""

    def __init__(self, *args, **kwds):
        warnings.warn("ExpertPRSideTrackingValidationModule is depricated for PRSideTrackingValidationModule",
                      DeprecationWarning)
        super().__init__(*args, **kwds)
