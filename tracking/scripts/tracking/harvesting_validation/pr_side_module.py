
import ROOT
from ROOT import Belle2


import numpy as np

import tracking.harvest.harvesting as harvesting
import tracking.harvest.refiners as refiners
import tracking.validation.utilities as utilities

from tracking.validation.utilities import getObjectList

import tracking.harvest.peelers as peelers
ROOT.gSystem.Load("libtracking")


class PRSideTrackingValidationModule(harvesting.HarvestingModule):
    """Module to collect matching information about the found particles and to generate
       validation plots and figures of merit on the performance of track finding."""

    """ Expert level behavior:
        expert_level <= default_expert_level: all figures and plots from this module except tree entries
        expert_level > default_expert_level: everything including tree entries
    """
    #: the threshold value for the expert level
    default_expert_level = 10

    def __init__(
            self,
            name,
            contact,
            output_file_name=None,
            reco_tracks_name='RecoTracks',
            mc_reco_tracks_name='MCRecoTracks',
            expert_level=None):
        """Constructor"""

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

        #: Cache for the MC hit lookup
        self.mc_hit_lookup = Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance()

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

        self.mc_hit_lookup.fill()

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

        # Peel function to get hit purity of subdetectors
        subdetector_hit_purity_crops = peelers.peel_subdetector_hit_purity(reco_track, mc_reco_track)

        # Information on TrackFinders
        trackfinder_crops = peelers.peel_trackfinder(reco_track)

        # Basic peel function to get Quality Indicators
        qualityindicator_crops = peelers.peel_quality_indicators(reco_track)

        # Get the fit results
        seed_fit_crops = peelers.peel_reco_track_seed(reco_track)

        fit_result = track_match_look_up.getRelatedTrackFitResult(reco_track)
        fit_crops = peelers.peel_track_fit_result(fit_result)
        fit_status_crops = peelers.peel_fit_status(reco_track)

        correct_rl_information = sum(peelers.is_correct_rl_information(cdc_hit, reco_track, self.mc_hit_lookup)
                                     for cdc_hit in getObjectList(reco_track.getCDCHitList()))

        crops = dict(
            correct_rl_information=correct_rl_information,
            **mc_particle_crops,
            **hit_content_crops,
            **pr_to_mc_match_info_crops,
            **subdetector_hit_purity_crops,  # Custom
            **trackfinder_crops,
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
        """Extracts track-match information from the MCMatcherTracksModule results"""
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

        mc_particle = track_match_look_up.getRelatedMCParticle(reco_track)
        mc_is_primary = False
        if mc_particle:
            mc_is_primary = bool(mc_particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle))

        return dict(
            is_matched=is_matched,
            is_matchedPrimary=is_matched and mc_is_primary,
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

    #: Save a tree of all collected variables in a sub folder
    save_tree = refiners.save_tree(
        #: \cond
        folder_name="pr_tree", name="pr_tree", above_expert_level=default_expert_level
        #: \endcond
    )

    #: Save RecoTrack clone-rate information
    save_clone_rate = refiners.save_fom(
        #: \cond
        name="{module.id}_overview_figures_of_merit",
        # Same as in the mc side module to combine the overview figures of merit into the same TNTuple
        title="Overview figures in {module.title}",
        description="clone_rate - ratio of clones divided the number of tracks that are related to a particle (clones and matches)",
        key="clone rate",
        select=["is_clone"],
        aggregation=np.mean,
        filter_on="is_clone_or_match",
        #: \endcond
    )

    #: Make profile of the clone rate versus seed tan(lambda)
    #: Rename the quantities to names that display nicely by root latex translation
    save_clone_rate_by_seed_tan_lambda_profile = refiners.save_profiles(
        #: \cond
        filter_on="is_clone_or_match",
        select={
            'is_clone': 'clone rate',
            'seed_tan_lambda_estimate': 'seed tan #lambda',
        },
        y='clone rate',
        y_binary=True,
        outlier_z_score=5.0,
        lower_bound=-1.73,
        upper_bound=3.27,
        bins=50
        #: \endcond
    )

    #: Make profile of the clone rate versus seed phi0
    #: Rename the quantities to names that display nicely by root latex translation
    save_clone_rate_by_seed_phi0_profile = refiners.save_profiles(
        #: \cond
        select={
            'is_clone': 'clone rate',
            'seed_phi0_estimate': 'seed #phi',
        },
        y='clone rate',
        y_binary=True,
        outlier_z_score=5.0,
        bins=50
        #: \endcond
    )

    #: Make profile of the clone rate versus seed transverse momentum
    #: Rename the quantities to names that display nicely by root latex translation
    save_clone_rate_by_seed_pt_profile = refiners.save_profiles(
        #: \cond
        filter_on="is_clone_or_match",
        select={
            'is_clone': 'clone rate',
            'seed_pt_estimate': 'seed p_{t}',
        },
        y='clone rate',
        y_binary=True,
        outlier_z_score=5.0,
        lower_bound=0,
        upper_bound=1.7,
        bins=50
        #: \endcond
    )

    #: Charge dependent histograms
    #: Make profile of the clone rate versus seed transverse momentum
    save_clone_rate_by_seed_pt_profile_groupbyCharge = refiners.save_profiles(
        #: \cond
        filter_on="is_clone_or_match",
        select={
            'is_clone': 'clone rate',
            'seed_pt_estimate': 'seed p_{t}',
        },
        y='clone rate',
        y_binary=True,
        groupby=[("charge_truth", [0.])],
        outlier_z_score=5.0,
        lower_bound=0,
        upper_bound=1.7,
        bins=50
        #: \endcond
    )

    #: Charge dependent histograms
    #: Make profile of the clone rate versus seed tan(lambda)
    save_clone_rate_by_seed_tan_lambda_profile_groupbyCharge = refiners.save_profiles(
        #: \cond
        filter_on="is_clone_or_match",
        select={
            'is_clone': 'clone rate',
            'seed_tan_lambda_estimate': 'seed tan #lambda',
        },
        y='clone rate',
        y_binary=True,
        groupby=[("charge_truth", [0.])],
        outlier_z_score=5.0,
        lower_bound=-1.73,
        upper_bound=3.27,
        bins=50
        #: \endcond
    )

    #: Save RecoTrack fake-rate information
    save_fake_rate = refiners.save_fom(
        #: \cond
        name="{module.id}_overview_figures_of_merit",
        # Same as in the mc side module to combine the overview figures of merit into the same TNTuple
        title="Overview figures in {module.title}",
        description="fake_rate - ratio of pattern recognition tracks that are not related to a particle" +
                    "(background, ghost) to all pattern recognition tracks",
        key="fake rate",
        select="is_fake",
        aggregation=np.mean,
        #: \endcond
    )

    #: Make profile of the fake rate versus seed phi0
    #: Rename the quantities to names that display nicely by root latex translation
    save_fake_rate_by_seed_phi0_profile = refiners.save_profiles(
        #: \cond
        select={
            'is_fake': 'fake rate',
            'seed_phi0_estimate': 'seed #phi',
        },
        y='fake rate',
        y_binary=True,
        outlier_z_score=5.0,
        #: \endcond
    )

    #: Make profile of the fake rate versus seed tan(lambda)
    #: Rename the quantities to names that display nicely by root latex translation
    save_fake_rate_by_seed_tan_lambda_profile = refiners.save_profiles(
        #: \cond
        select={
            'is_fake': 'fake rate',
            'seed_tan_lambda_estimate': 'seed tan #lambda',
        },
        y='fake rate',
        y_binary=True,
        outlier_z_score=5.0,
        lower_bound=-1.73,
        upper_bound=3.27,
        #: \endcond
    )

    #: Make profile of the fake rate versus seed transverse momentum
    #: Rename the quantities to names that display nicely by root latex translation
    save_fake_rate_by_seed_pt_profile = refiners.save_profiles(
        #: \cond
        select={
            'is_fake': 'fake rate',
            'seed_pt_estimate': 'seed p_{t}',
        },
        y='fake rate',
        y_binary=True,
        outlier_z_score=5.0,
        lower_bound=0,
        upper_bound=1.7,
        #: \endcond
    )

    #: Charge dependent histograms
    #: Make profile of the fake rate versus seed tan(lambda)
    save_fake_rate_by_seed_tan_lambda_profile_groupbyCharge = refiners.save_profiles(
        #: \cond
        filter_on="has_trackFitResult",
        select={
            'is_fake': 'fake rate',
            'seed_tan_lambda_estimate': 'seed tan #lambda',
        },
        y='fake rate',
        y_binary=True,
        outlier_z_score=5.0,
        lower_bound=-1.73,
        upper_bound=3.27,
        groupby=[("track_charge", [0.])],
        #: \endcond
    )

    #: Charge dependent histograms
    #: Make profile of the fake rate versus seed transverse momentum
    save_fake_rate_by_seed_pt_profile_groupbyCharge = refiners.save_profiles(
        #: \cond
        filter_on="has_trackFitResult",
        select={
            'is_fake': 'fake rate',
            'seed_pt_estimate': 'seed p_{t}',
        },
        y='fake rate',
        y_binary=True,
        outlier_z_score=5.0,
        lower_bound=0,
        upper_bound=1.7,
        groupby=[("track_charge", [0.])],
        #: \endcond
    )

    #: Hit counts in each sub detector by the true pt value
    save_hit_counts_by_pt_profile = refiners.save_profiles(
        #: \cond
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
        ],
        outlier_z_score=5.0,
        lower_bound=0,
        upper_bound=1.7,
        #: \endcond
    )

    #: Hit efficiency in each sub detector by the true pt value
    save_hit_efficiency_by_pt_profile = refiners.save_profiles(
        #: \cond
        filter_on="is_matchedPrimary",
        select={
            "pt_truth": "true p_{t}",
            "mc_pxd_hit_efficiency": "pxd hit efficiency",
            "mc_svd_hit_efficiency": "svd hit efficiency",
            "mc_cdc_hit_efficiency": "cdc hit efficiency",
        },
        y=[
            "pxd hit efficiency",
            "svd hit efficiency",
            "cdc hit efficiency",
        ],
        outlier_z_score=5.0,
        lower_bound=0,
        upper_bound=1.7,
        #: \endcond
    )

    #: Hit purity in each sub detector by the true pt value
    save_hit_purity_by_pt_profile = refiners.save_profiles(
        #: \cond
        filter_on="is_matchedPrimary",
        select={
            "pt_truth": "true p_{t}",
            "pxd_hit_purity": "pxd hit purity",
            "svd_hit_purity": "svd hit purity",
            "cdc_hit_purity": "cdc hit purity",
        },
        y=[
            "pxd hit purity",
            "svd hit purity",
            "cdc hit purity",
        ],
        outlier_z_score=5.0,
        lower_bound=0,
        upper_bound=1.7,
        #: \endcond
    )

    #: Hit counts in each sub detector by the true tanlambda value
    save_hit_counts_by_tanlambda_profile = refiners.save_profiles(
        #: \cond
        filter_on="is_matched",
        select={
            "tan_lambda_truth": "true tan #lambda",
            "n_pxd_hits": "pxd hits",
            "n_svd_hits": "svd hits",
            "n_cdc_hits": "cdc hits",
        },
        y=[
            "pxd hits",
            "svd hits",
            "cdc hits",
        ],
        outlier_z_score=5.0,
        lower_bound=-1.73,
        upper_bound=3.27,
        #: \endcond
    )

    #: Hit efficiency in each sub detector by the true tanlambda value
    save_hit_efficiency_by_tanlambda_profile = refiners.save_profiles(
        #: \cond
        filter_on="is_matchedPrimary",
        select={
            "tan_lambda_truth": "true tan #lambda",
            "mc_pxd_hit_efficiency": "pxd hit efficiency",
            "mc_svd_hit_efficiency": "svd hit efficiency",
            "mc_cdc_hit_efficiency": "cdc hit efficiency",
        },
        y=[
            "pxd hit efficiency",
            "svd hit efficiency",
            "cdc hit efficiency",
        ],
        outlier_z_score=5.0,
        lower_bound=-1.73,
        upper_bound=3.27,
        #: \endcond
    )

    #: Hit purity in each sub detector by the true tanlambda value
    save_hit_purity_by_tanlambda_profile = refiners.save_profiles(
        #: \cond
        filter_on="is_matchedPrimary",
        select={
            "tan_lambda_truth": "true tan #lambda",
            "pxd_hit_purity": "pxd hit purity",
            "svd_hit_purity": "svd hit purity",
            "cdc_hit_purity": "cdc hit purity",
        },
        y=[
            "pxd hit purity",
            "svd hit purity",
            "cdc hit purity",
        ],
        outlier_z_score=5.0,
        lower_bound=-1.73,
        upper_bound=3.27,
        #: \endcond
    )

    #: Charge dependent histograms
    #: Hit counts in each sub detector by the true pt value
    save_hit_counts_by_pt_profile_groupbyCharge = refiners.save_profiles(
        #: \cond
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
        ],
        groupby=[("charge_truth", [0.])],
        outlier_z_score=5.0,
        lower_bound=0,
        upper_bound=1.7,
        #: \endcond
    )

    #: Charge dependent histograms
    #: Hit counts in each sub detector by the true tan lambda value
    save_hit_counts_by_tanlambda_profile_groupbyCharge = refiners.save_profiles(
        #: \cond
        filter_on="is_matched",
        select={
            "tan_lambda_truth": "true tan #lambda",
            "n_pxd_hits": "pxd hits",
            "n_svd_hits": "svd hits",
            "n_cdc_hits": "cdc hits",
        },
        y=[
            "pxd hits",
            "svd hits",
            "cdc hits",
        ],
        groupby=[("charge_truth", [0.])],
        outlier_z_score=5.0,
        lower_bound=-1.73,
        upper_bound=3.27,
        #: \endcond
    )

    #: Charge dependent histograms
    #: Hit efficiency in each sub detector by the true pt value
    save_hit_efficiency_by_pt_profile_groupbyCharge = refiners.save_profiles(
        #: \cond
        filter_on="is_matchedPrimary",
        select={
            "pt_truth": "true p_{t}",
            "mc_pxd_hit_efficiency": "pxd hit efficiency",
            "mc_svd_hit_efficiency": "svd hit efficiency",
            "mc_cdc_hit_efficiency": "cdc hit efficiency",
        },
        y=[
            "pxd hit efficiency",
            "svd hit efficiency",
            "cdc hit efficiency",
        ],
        groupby=[("charge_truth", [0.])],
        outlier_z_score=5.0,
        lower_bound=0,
        upper_bound=1.7,
        #: \endcond
    )

    #: Charge dependent histograms
    #: Hit efficiency in each sub detector by the true tan lambda value
    save_hit_efficiency_by_tanlambda_profile = refiners.save_profiles(
        #: \cond
        filter_on="is_matchedPrimary",
        select={
            "tan_lambda_truth": "true tan #lambda",
            "mc_pxd_hit_efficiency": "pxd hit efficiency",
            "mc_svd_hit_efficiency": "svd hit efficiency",
            "mc_cdc_hit_efficiency": "cdc hit efficiency",
        },
        y=[
            "pxd hit efficiency",
            "svd hit efficiency",
            "cdc hit efficiency",
        ],
        groupby=[("charge_truth", [0.])],
        outlier_z_score=5.0,
        lower_bound=-1.73,
        upper_bound=3.27,
        #: \endcond
    )

    #: Save simple FOM
    save_hit_efficiency = refiners.save_fom(
        #: \cond
        name="{module.id}_subdetector_figures_of_merit",
        title="Overview figures in {module.title}",
        description="Hit efficiency in the subdetectors",
        key="hit efficiency",
        select="mc_hit_efficiency",
        aggregation=np.nanmean,
        filter_on="is_matchedPrimary"
        #: \endcond
    )

    #: Save simple FOM
    save_pxd_hit_efficiency = refiners.save_fom(
        #: \cond
        name="{module.id}_subdetector_figures_of_merit",
        title="Overview figures in {module.title}",
        description="Hit efficiency in the subdetectors",
        key="pxd hit efficiency",
        select="mc_pxd_hit_efficiency",
        aggregation=np.nanmean,
        filter_on="is_matchedPrimary"
        #: \endcond
    )

    #: Save simple FOM
    save_svd_hit_efficiency = refiners.save_fom(
        #: \cond
        name="{module.id}_subdetector_figures_of_merit",
        title="Overview figures in {module.title}",
        description="Hit efficiency in the subdetectors",
        key="svd hit efficiency",
        select="mc_svd_hit_efficiency",
        aggregation=np.nanmean,
        filter_on="is_matchedPrimary"
        #: \endcond
    )

    #: Save simple FOM
    save_cdc_hit_efficiency = refiners.save_fom(
        #: \cond
        name="{module.id}_subdetector_figures_of_merit",
        title="Overview figures in {module.title}",
        description="Hit efficiency in the subdetectors",
        key="cdc hit efficiency",
        select="mc_cdc_hit_efficiency",
        aggregation=np.nanmean,
        filter_on="is_matchedPrimary"
        #: \endcond
    )

    #: Save simple FOM
    save_hit_purity = refiners.save_fom(
        #: \cond
        name="{module.id}_subdetector_figures_of_merit",
        title="Overview figures in {module.title}",
        description="Hit purity in the subdetectors",
        key="hit purity",
        select="hit_purity",
        aggregation=np.nanmean,
        filter_on="is_matchedPrimary"
        #: \endcond
    )

    #: Save simple FOM
    save_pxd_hit_purity = refiners.save_fom(
        #: \cond
        name="{module.id}_subdetector_figures_of_merit",
        title="Overview figures in {module.title}",
        description="Hit purity in the subdetectors",
        key="pxd hit purity",
        select="pxd_hit_purity",
        aggregation=np.nanmean,
        filter_on="is_matchedPrimary"
        #: \endcond
    )

    #: Save simple FOM
    save_svd_hit_purity = refiners.save_fom(
        #: \cond
        name="{module.id}_subdetector_figures_of_merit",
        title="Overview figures in {module.title}",
        description="Hit purity in the subdetectors",
        key="svd hit purity",
        select="svd_hit_purity",
        aggregation=np.nanmean,
        filter_on="is_matchedPrimary"
        #: \endcond
    )

    #: Save simple FOM
    save_cdc_hit_purity = refiners.save_fom(
        #: \cond
        name="{module.id}_subdetector_figures_of_merit",
        title="Overview figures in {module.title}",
        description="Hit purity in the subdetectors",
        key="cdc hit purity",
        select="cdc_hit_purity",
        aggregation=np.nanmean,
        filter_on="is_matchedPrimary"
        #: \endcond
    )

    #: Creates a distribution of p values from the Genfit track fit for match pr tracks.
    save_p_value_histogram = refiners.save_histograms(
        #: \cond
        filter_on="is_matched",
        select={"p_value": "Genfit p value"},
        description="""
                    The distribution of p values from the Genfit track fit.
                    If all errors are propagated correctly the distribution should be flat.
                    Generally some peaking behvaiour towards zero is too be expected if the errors are underestimated.
                    """,
        check="The distribution should be flat."
        #: \endcond
    )

    #: Pull of seed omega
    save_seed_omega_pull_analysis = refiners.save_pull_analysis(
        #: \cond
        filter_on="is_matched",
        part_name="seed_omega",
        quantity_name="seed #omega",
        folder_name="pull_seed_omega",
        truth_name="omega_truth",
        unit="1/cm",
        #: \endcond
    )

    #: Pull of seed tan(lambda)
    save_seed_tan_lambda_pull_analysis = refiners.save_pull_analysis(
        #: \cond
        filter_on="is_matched",
        part_name="seed_tan_lambda",
        quantity_name="seed tan #lambda",
        folder_name="pull_seed_tan_lambda",
        truth_name="tan_lambda_truth",
        #: \endcond
    )

    #: Pull of fitted omega
    save_fitted_omega_pull_analysis = refiners.save_pull_analysis(
        #: \cond
        filter_on="is_matched",
        part_name="omega",
        quantity_name="#omega",
        folder_name="pull_fitted_omega",
        unit="1/cm",
        #: \endcond
    )

    #: Pull of fitted tan(lambda)
    save_fitted_tan_lambda_pull_analysis = refiners.save_pull_analysis(
        #: \cond
        filter_on="is_matched",
        part_name="tan_lambda",
        quantity_name="tan #lambda",
        folder_name="pull_fitted_tan_lambda",
        #: \endcond
    )

    #: Pull of fitted transverse momentum
    save_fitted_pt_pull_analysis = refiners.save_pull_analysis(
        #: \cond
        filter_on="is_matched",
        part_name="pt",
        quantity_name="p_{t}",
        folder_name="pull_fitted_p_t",
        #: \endcond
    )

    #: Pull of fitted x coordinate grouped by true transverse momentum
    save_fitted_x_pull_analysis = refiners.save_pull_analysis(
        #: \cond
        filter_on="is_matched",
        part_name="x",
        quantity_name="x",
        folder_name="pull_fitted_x{groupby_addition}",
        groupby=[None, ("pt_truth", [0.070, 0.250, 0.600])],
        #: \endcond
    )

    #: Pull of fitted y coordinate grouped by true transverse momentum
    save_fitted_y_pull_analysis = refiners.save_pull_analysis(
        #: \cond
        filter_on="is_matched",
        part_name="y",
        quantity_name="y",
        folder_name="pull_fitted_y{groupby_addition}",
        groupby=[None, ("pt_truth", [0.070, 0.250, 0.600])],
        #: \endcond
    )

    #: Pull of fitted z coordinate grouped by true transverse momentum
    save_fitted_z_pull_analysis = refiners.save_pull_analysis(
        #: \cond
        filter_on="is_matched",
        part_name="z",
        quantity_name="z",
        folder_name="pull_fitted_z{groupby_addition}",
        groupby=[None, ("pt_truth", [0.070, 0.250, 0.600])],
        #: \endcond
    )

    #: Resolutions as a function of true p_t
    save_resolutions_by_pt_profile = refiners.save_profiles(
        #: \cond
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
        #: \endcond
    )
