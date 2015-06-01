from tracking.validation.harvesting import HarvestingModule
from tracking.validation import refiners

from ROOT import Belle2

import numpy as np


class ReconstructionPositionHarvester(HarvestingModule):
    #: Harvester module to check for the reconstructed positions

    def __init__(self, output_file_name, tracks_store_vector_name="CDCTrackVector"):
        super(
            ReconstructionPositionHarvester,
            self).__init__(
            foreach=tracks_store_vector_name,
            output_file_name=output_file_name)

        self.mc_hit_lookup = Belle2.TrackFindingCDC.CDCMCHitLookUp()

    def peel(self, track_cand):

        mc_hit_lookup = self.mc_hit_lookup
        mc_hit_lookup.fill()

        sum_of_difference_norms_stereo = 0
        num_norms_stereo = 0
        sum_of_difference_norms_axial = 0
        num_norms_axial = 0

        number_of_wrong_rl_infos = 0

        for reco_hit in track_cand.items():
            underlaying_cdc_hit = reco_hit.getWireHit().getHit()
            hit_difference = mc_hit_lookup.getRecoPos3D(underlaying_cdc_hit) - reco_hit.getRecoPos3D()
            sum_of_difference_norms_axial += hit_difference.xy().norm()
            num_norms_axial += 1

            if reco_hit.getStereoType() != 0:  # AXIAL
                sum_of_difference_norms_stereo += abs(hit_difference.z())
                num_norms_stereo += 1

                correct_rl_info = mc_hit_lookup.getRLInfo(underlaying_cdc_hit)

                if correct_rl_info != reco_hit.getRLInfo():
                    number_of_wrong_rl_infos += 1

        return dict(sum_of_difference_norms_axial=sum_of_difference_norms_axial,
                    num_norms_axial=num_norms_axial,
                    mean_of_difference_norms_axial=np.true_divide(sum_of_difference_norms_axial, num_norms_axial),
                    sum_of_difference_norms_stereo=sum_of_difference_norms_stereo,
                    num_norms_stereo=num_norms_stereo,
                    mean_of_difference_norms_stereo=np.true_divide(sum_of_difference_norms_stereo, num_norms_stereo),
                    number_of_wrong_rl_infos=number_of_wrong_rl_infos,
                    mean_wrong_rl_infos=np.true_divide(number_of_wrong_rl_infos, num_norms_stereo))

    save_tree = refiners.save_tree(folder_name="tree")
