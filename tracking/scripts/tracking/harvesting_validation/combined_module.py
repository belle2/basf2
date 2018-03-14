import ROOT
from tracking import metamodules

from .mc_side_module import MCSideTrackingValidationModule
from .pr_side_module import PRSideTrackingValidationModule
from .eventwise_module import EventwiseTrackingValidationModule


class CombinedTrackingValidationModule(metamodules.PathModule):
    """Harvesting validation module combining eventwise, pr side and mc side tracking valiation parts"""

    #: MC side validation module to use
    MCSideModule = MCSideTrackingValidationModule

    #: PR side validation module to use
    PRSideModule = PRSideTrackingValidationModule

    #: Eventwise side validation module to use
    EventwiseModule = EventwiseTrackingValidationModule

    def __init__(self,
                 name,
                 contact,
                 output_file_name=None,
                 reco_tracks_name='RecoTracks',
                 mc_reco_tracks_name='MCRecoTracks',
                 expert_level=None):
        """Constructor"""

        #: Output TFile to be opened in the initialize methode
        self.output_file_name = output_file_name or name + 'TrackingValidation.root'

        # First forward the output_file_name to the separate modules
        # but we open the TFile in the top level and forward it to them on initialize.

        eventwise_module = self.EventwiseModule(name,
                                                contact,
                                                output_file_name=self.output_file_name,
                                                reco_tracks_name=reco_tracks_name,
                                                mc_reco_tracks_name=mc_reco_tracks_name,
                                                expert_level=expert_level)

        pr_side_module = self.PRSideModule(name,
                                           contact,
                                           output_file_name=self.output_file_name,
                                           reco_tracks_name=reco_tracks_name,
                                           mc_reco_tracks_name=mc_reco_tracks_name,
                                           expert_level=expert_level)

        mc_side_module = self.MCSideModule(name,
                                           contact,
                                           output_file_name=self.output_file_name,
                                           reco_tracks_name=reco_tracks_name,
                                           mc_reco_tracks_name=mc_reco_tracks_name,
                                           expert_level=expert_level)

        #: Subordinate modules
        self.modules = [
            eventwise_module,
            pr_side_module,
            mc_side_module,
        ]

        super().__init__(modules=self.modules)

    def initialize(self):
        """Initialization signal at the start of the event processing"""
        super().initialize()
        output_tfile = ROOT.TFile(self.output_file_name, "RECREATE")

        # Substitute the output file with the common output file opened in this module.
        for module in self.modules:
            module.output_file_name = output_tfile

        self.output_tfile = output_tfile

    def terminate(self):
        """Termination signal at the start of the event processing"""
        self.output_tfile.Flush()
        self.output_tfile.Close()
        super().terminate()
