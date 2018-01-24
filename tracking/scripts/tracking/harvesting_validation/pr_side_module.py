import basf2

import ROOT
ROOT.gSystem.Load("libtracking")
from ROOT import Belle2

import math
import warnings

import numpy as np

import tracking.harvest.harvesting as harvesting
import tracking.harvest.refiners as refiners
import tracking.validation.utilities as utilities

import tracking.harvest.peelers as peelers


class PRSideTrackingValidationModule(harvesting.HarvestingModule):
    """Module to collect matching information about the found particles and to generate
       validation plots and figures of merit on the performance of track finding."""

    """ Expert level behavior:
        expert_level <= default_expert_level: all figures and plots from this module except tree entries
        expert_level > default_expert_level: everything including tree entries
    """
    default_expert_level = 10

    def __init__(
            self,
            name,
            contact,
            output_file_name=None,
            reco_tracks_name='RecoTracks',
            mc_reco_tracks_name='MCRecoTracks',
            expert_level=None):

        output_file_name = output_file_name or name + 'TrackingValidation.root'
        super().__init__(foreach=reco_tracks_name,
                         name=name,
                         contact=contact,
                         output_file_name=output_file_name,
                         expert_level=expert_level)

        #: Name of the StoreArray of the tracks from pattern recognition
        self.reco_tracks_name = reco_tracks_name

        #: Name of the StoreArray of the ideal mc tracks
        self.mc_reco_tracks_name = mc_reco_tracks_name

        #: Reference to the track match lookup object reading the relation information constructed by the MCMatcherTracksModule
        self.track_match_look_up = None

        #: Cache for the hit content of the Monte Carlo tracks - updated each event
        self.mc_reco_tracks_det_hit_ids = []

    def initialize(self):
        """Receive signal at the start of event processing"""
        super().initialize()
        self.track_match_look_up = Belle2.TrackMatchLookUp(self.mc_reco_tracks_name,
                                                           self.reco_tracks_name)

    def prepare(self):
        """Called once at the start of each event"""
        super().prepare()
        mc_reco_tracks = Belle2.PyStoreArray(self.mc_reco_tracks_name)
        mc_reco_tracks_det_hit_ids = []

        for mc_reco_track in mc_reco_tracks:
            mc_reco_track_det_hit_ids = utilities.get_det_hit_ids(mc_reco_track)
            mc_reco_tracks_det_hit_ids.append(mc_reco_track_det_hit_ids)

        self.mc_reco_tracks_det_hit_ids = mc_reco_tracks_det_hit_ids

    def pick(self, reco_track):
        """Method to filter the track candidates to reject part of them"""
        return True

    def peel(self, reco_track):
        """Looks at the individual pattern recognition tracks and store information about them"""
        track_match_look_up = self.track_match_look_up

        # Matching information
        mc_reco_track = track_match_look_up.getRelatedMCRecoTrack(reco_track)
        mc_particle = track_match_look_up.getRelatedMCParticle(reco_track)
        mc_particle_crops = peelers.peel_mc_particle(mc_particle)

        hit_content_crops = peelers.peel_reco_track_hit_content(reco_track)

        pr_to_mc_match_info_crops = self.peel_pr_to_mc_match_info(reco_track)

        # Custom peel function to get hit purity of subdetectors
        subdetector_hit_purity_crops = peelers.peel_subdetector_hit_purity(reco_track, mc_reco_track)

        # Basic peel function to get Quality Indicators
        qualityindicator_crops = peelers.peel_quality_indicators(reco_track)

        # Get the fit results
        seed_fit_crops = peelers.peel_reco_track_seed(reco_track)

        fit_result = track_match_look_up.getRelatedTrackFitResult(reco_track)
        fit_crops = peelers.peel_track_fit_result(fit_result)
        fit_status_crops = peelers.peel_fit_status(reco_track)

        crops = dict(
            **mc_particle_crops,
            **hit_content_crops,
            **pr_to_mc_match_info_crops,
            **subdetector_hit_purity_crops,  # Custom
            **qualityindicator_crops,
            **seed_fit_crops,
            **fit_crops,
            **fit_status_crops,
        )

        if self.expert_level >= self.default_expert_level:

            # Event Info
            event_meta_data = Belle2.PyStoreObj("EventMetaData")
            event_crops = peelers.peel_event_info(event_meta_data)

            # Store Array for easier joining
            store_array_crops = peelers.peel_store_array_info(reco_track, key="pr_{part_name}")
            mc_store_array_crops = peelers.peel_store_array_info(mc_reco_track, key="mc_{part_name}")

            # Information on PR reco track
            mc_efficiency_information = {
                "mc_hit_efficiency": track_match_look_up.getRelatedEfficiency(mc_reco_track) if mc_reco_track else float("nan"),
                **peelers.peel_subdetector_hit_efficiency(reco_track=reco_track, mc_reco_track=mc_reco_track,
                                                          key="mc_{part_name}")
            }

            crops.update(
                **event_crops,
                **store_array_crops,
                **mc_store_array_crops,
                **mc_efficiency_information
            )

        return crops

    def peel_pr_to_mc_match_info(self, reco_track):
        track_match_look_up = self.track_match_look_up
        is_matched = track_match_look_up.isMatchedPRRecoTrack(reco_track)
        is_clone = track_match_look_up.isClonePRRecoTrack(reco_track)
        is_background = track_match_look_up.isBackgroundPRRecoTrack(reco_track)
        is_ghost = track_match_look_up.isGhostPRRecoTrack(reco_track)

        reco_track_det_hit_ids = utilities.get_det_hit_ids(reco_track)
        n_intersecting_mc_tracks = 0
        for mc_reco_track_det_hit_ids in self.mc_reco_tracks_det_hit_ids:
            intersects = len(mc_reco_track_det_hit_ids & reco_track_det_hit_ids) > 0
            if intersects:
                n_intersecting_mc_tracks += 1

        return dict(
            is_matched=is_matched,
            is_clone=is_clone,
            is_background=is_background,
            is_ghost=is_ghost,
            is_clone_or_match=(is_matched or is_clone),
            is_fake=not (is_matched or is_clone),
            hit_purity=track_match_look_up.getRelatedPurity(reco_track),
            n_intersecting_mc_tracks=n_intersecting_mc_tracks,
        )

    # Refiners to be executed on terminate #
    # #################################### #

    # Save a tree of all collected variables in a sub folder
    save_tree = refiners.save_tree(folder_name="pr_tree", name="pr_tree", above_expert_level=default_expert_level)

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

    save_fake_rate_by_seed_pt_profile = refiners.save_profiles(
        select={
            'is_fake': 'fake rate',
            'seed_pt_estimate': 'seed p_{t}',
        },
        y='fake rate',
        y_binary=True,
        outlier_z_score=5.0,
        lower_bound=0,
        upper_bound=1.7,
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
