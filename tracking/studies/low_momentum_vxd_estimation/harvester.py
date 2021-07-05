##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from tracking.harvest.harvesting import HarvestingModule
from tracking.harvest import refiners
from ROOT import Belle2
import numpy as np

from tracking.ipython_tools.wrapper import QueueHarvester


class VXDMomentumEnergyEstimator:

    """ The base class with all static methods to use. """

    @staticmethod
    def do_for_each_hit_type(cluster, svd_function, pxd_function):
        """Apply a PXD selection to a PXDCluster or an SVD selection to an SVDCluster"""
        cluster_type = cluster.__class__.__name__
        if cluster_type == "Belle2::PXDCluster":
            return pxd_function(cluster)
        elif cluster_type == "Belle2::SVDCluster":
            return svd_function(cluster)
        else:
            raise TypeError("Unknown hit type")

    @staticmethod
    def calculate_charges_and_path_lengths_for_one_type(clusters, mc_particle):
        """Return lists of charges and path lengths for the clusters associated with an MCParticle"""
        charge_list = []
        path_length_list = []

        tools = Belle2.VXDMomentumEstimationTools(clusters[0].__class__.__name__).getInstance()

        momentum = mc_particle.getMomentum()
        position = mc_particle.getProductionVertex()
        charge = mc_particle.getCharge()
        b_field = Belle2.BFieldManager.getField(position).Z() / Belle2.Unit.T
        helix = Belle2.Helix(position, momentum, charge, b_field)

        for cluster in clusters:

            calibrated_charge = tools.getCalibratedCharge(cluster)
            path_length = tools.getPathLength(cluster, helix)

            charge_list.append(calibrated_charge)
            path_length_list.append(path_length)

        return charge_list, path_length_list, list(np.divide(charge_list, path_length_list))

    @staticmethod
    def generate_truncated(charge_list):
        """Sort then truncate a list to all but the last 2 entries or the first 4 entries or the first 6 entries"""
        sorted_list = sorted(charge_list)
        if len(sorted_list) > 2:
            return sorted_list[:-2], sorted_list[:4], sorted_list[:6]
        else:
            return sorted_list, sorted_list[:4], sorted_list[:6]


class MCTrajectoryHarvester(HarvestingModule):

    """ A harvester to check for the positions of the track points in the MCParticleTrajectories"""

    def __init__(self):
        """Constructor"""
        HarvestingModule.__init__(self, foreach="MCParticleTrajectorys", output_file_name="mc_trajectory.root")

    def peel(self, mc_particle_trajectory):
        """Aggregate track-point position information for the trajectory"""
        for track_point in mc_particle_trajectory:
            yield {"x": track_point.x, "y": track_point.y, "z": track_point.z, "index": self.counter}

    #: Refiners to be executed at the end of the harvesting / termination of the module
    #: Save a tree of all collected variables in a sub folder
    save_tree = refiners.SaveTreeRefiner()


class MCParticleHarvester(HarvestingModule):

    """ A harvester to redo parts of the analysis in the Belle II Paper by Robert """

    def __init__(self):
        """Constructor"""
        HarvestingModule.__init__(self, foreach="MCParticles", output_file_name="mc_particle.root")

    def pick(self, mc_particle):
        """Select the MCParticle if it is a primary pion and has some PXD and/or SVD clusters"""
        pxd_clusters = mc_particle.getRelationsFrom("PXDClusters")
        svd_clusters = mc_particle.getRelationsFrom("SVDClusters")
        return (mc_particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle) and
                abs(mc_particle.getPDG()) == 211 and
                len(pxd_clusters) + len(svd_clusters) > 0)

    def generate_cluster_dicts(self, charge_list, path_length_list, normalized_charge_list, name):
        """Create a dictionary from the lists of charges, normalized charges, and path lengths of the
           clusters associated with an MCParticle"""
        result = dict()

        truncated, first4, first6 = VXDMomentumEnergyEstimator.generate_truncated(normalized_charge_list)

        result.update({"sum_%s_charges" % name: sum(charge_list)})
        result.update({"mean_%s_charges" % name: np.mean(charge_list)})
        result.update({"sum_%s_normalized_charges" % name: sum(normalized_charge_list)})
        result.update({"mean_%s_normalized_charges" % name: np.mean(normalized_charge_list)})

        result.update({"sum_%s_normalized_charges_truncated" % name: sum(truncated)})
        result.update({"mean_%s_normalized_charges_truncated" % name: np.mean(truncated)})
        result.update({"sum_%s_normalized_charges_first4" % name: sum(first4)})
        result.update({"mean_%s_normalized_charges_first4" % name: np.mean(first4)})
        result.update({"sum_%s_normalized_charges_first6" % name: sum(first6)})
        result.update({"mean_%s_normalized_charges_first6" % name: np.mean(first6)})

        return result

    def peel(self, mc_particle):
        """Aggregate the PXD and SVD cluster information for an MCParticle"""
        result = dict()

        pxd_clusters = mc_particle.getRelationsFrom("PXDClusters")
        svd_clusters = mc_particle.getRelationsFrom("SVDClusters")

        pxd_results = VXDMomentumEnergyEstimator.calculate_charges_and_path_lengths_for_one_type(pxd_clusters, mc_particle)
        svd_results = VXDMomentumEnergyEstimator.calculate_charges_and_path_lengths_for_one_type(svd_clusters, mc_particle)

        pxd_cluster_dicts = self.generate_cluster_dicts(*pxd_results, name="pxd")
        pxd_charges, pxd_path_length, pxd_normalized_charges = pxd_results

        svd_charges, svd_path_length, svd_normalized_charges = svd_results
        svd_cluster_dicts = self.generate_cluster_dicts(*svd_results, name="svd")

        combined_cluster_dicts = self.generate_cluster_dicts(pxd_charges + svd_charges,
                                                             pxd_path_length + svd_path_length,
                                                             pxd_normalized_charges + svd_normalized_charges,
                                                             name="combined")

        result.update(pxd_cluster_dicts)
        result.update(svd_cluster_dicts)
        result.update(combined_cluster_dicts)

        return result

    #: Refiners to be executed at the end of the harvesting / termination of the module
    #: Save a tree of all collected variables in a sub folder
    save_tree = refiners.SaveTreeRefiner()


class VXDHarvester(QueueHarvester):

    """ A base class for the VXD hitwise analysis. Collect dE/dX and the correct p of each hit of the MC particles. """

    def __init__(self, clusters, detector, output_file_name, use_mc_info=True):
        """Constructor"""
        HarvestingModule.__init__(self, foreach="TrackCands", output_file_name=output_file_name)

        #: cached accessor to the SVD-tools singleton
        self.svd_tools = Belle2.VXDMomentumEstimationTools("Belle2::SVDCluster").getInstance()
        #: cached accessor to the PXD-tools singleton
        self.pxd_tools = Belle2.VXDMomentumEstimationTools("Belle2::PXDCluster").getInstance()

        #: cached copy of the name of the cluster StoreArray
        self.clusters = clusters
        #: cached copy of the detector identifier (PXD or SVD)
        self.detector = detector

        #: if true the MC information is used
        self.use_mc_info = use_mc_info

    def is_valid_cluster(self, cluster):
        """Determine if a cluster has an associated SpacePoint and is associated with a primary pion MCParticle"""
        mc_particles = cluster.getRelationsTo("MCParticles")

        space_point = cluster.getRelated("SpacePoints")

        if space_point is None:
            return False

        for mc_particle in mc_particles:
            if (mc_particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle) and
                    abs(mc_particle.getPDG()) == 211):

                return True

        return False

    def get_tools(self, cluster):
        """Get the PXD tools for a PXD cluster or the SVD tools for an SVD cluster"""
        return VXDMomentumEnergyEstimator.do_for_each_hit_type(
            cluster,
            lambda cluster: self.svd_tools,
            lambda cluster: self.pxd_tools)

    def peel(self, track_cand):
        """Aggregate the cluster and MCParticle (for primary pion) information associated with the track candidate"""
        vxd_hit_ids = track_cand.getHitIDs(self.detector)

        vxd_hits = Belle2.PyStoreArray(self.clusters)

        for vxd_hit_id in vxd_hit_ids:
            cluster = vxd_hits[vxd_hit_id]

            if not self.is_valid_cluster(cluster):
                continue

            tools = self.get_tools(cluster)

            mc_particles = cluster.getRelationsTo("MCParticles")

            for mc_particle in mc_particles:
                if (mc_particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle) and abs(mc_particle.getPDG()) == 211):

                    if self.use_mc_info:
                        track_momentum = mc_particle.getMomentum()
                        track_position = mc_particle.getProductionVertex()
                        track_charge = mc_particle.getCharge()
                    else:
                        track_momentum = track_cand.getMomSeed()
                        track_position = track_cand.getPosSeed()
                        track_charge = track_cand.getChargeSeed()

                    b_field = Belle2.BFieldManager.getField(track_position).Z() / Belle2.Unit.T
                    track_helix = Belle2.Helix(track_position, track_momentum, int(track_charge), b_field)

                    cluster_charge = tools.getCalibratedCharge(cluster)
                    path_length = tools.getPathLength(cluster, track_helix)
                    cluster_thickness = tools.getThicknessOfCluster(cluster)
                    cluster_radius = tools.getRadiusOfCluster(cluster)
                    cluster_width = tools.getWidthOfCluster(cluster)
                    cluster_length = tools.getLengthOfCluster(cluster)

                    perp_s_at_cluster_entry = track_helix.getArcLength2DAtCylindricalR(cluster_radius)
                    perp_s_at_cluster_exit = track_helix.getArcLength2DAtCylindricalR(cluster_radius + cluster_thickness)

                    mc_momentum = tools.getEntryMomentumOfMCParticle(cluster)
                    mc_position = tools.getEntryPositionOfMCParticle(cluster)

                    mc_b_field = Belle2.BFieldManager.getField(mc_position).Z() / Belle2.Unit.T
                    mc_helix = Belle2.Helix(mc_position, mc_momentum, int(track_charge), mc_b_field)
                    mc_path_length = tools.getPathLength(cluster, mc_helix)

                    cluster_is_u = VXDMomentumEnergyEstimator.do_for_each_hit_type(
                        cluster,
                        lambda cluster: cluster.isUCluster(),
                        lambda cluster: np.NaN)

                    cluster_is_pxd = VXDMomentumEnergyEstimator.do_for_each_hit_type(
                        cluster,
                        lambda cluster: False,
                        lambda cluster: True)

                    cluster_layer = tools.getLayerOfCluster(cluster)
                    cluster_segment = tools.getSegmentNumberOfCluster(cluster)
                    cluster_ladder = tools.getLadderOfCluster(cluster)
                    cluster_sensor = tools.getSensorNumberOfCluster(cluster)

                    cluster_dict = dict(cluster_charge=cluster_charge,
                                        cluster_thickness=cluster_thickness,
                                        cluster_radius=cluster_radius,
                                        cluster_is_u=cluster_is_u,
                                        cluster_is_pxd=cluster_is_pxd,
                                        cluster_layer=cluster_layer,
                                        cluster_segment=cluster_segment,
                                        cluster_ladder=cluster_ladder,
                                        cluster_width=cluster_width,
                                        cluster_length=cluster_length,
                                        cluster_sensor=cluster_sensor)

                    mc_at_hit_dict = dict(mc_helix_perigee_x=mc_helix.getPerigeeX(),
                                          mc_helix_perigee_y=mc_helix.getPerigeeY(),
                                          mc_helix_perigee_z=mc_helix.getPerigeeZ(),
                                          mc_helix_momentum_x=mc_helix.getMomentumX(mc_b_field),
                                          mc_helix_momentum_y=mc_helix.getMomentumY(mc_b_field),
                                          mc_helix_momentum_z=mc_helix.getMomentumZ(mc_b_field),
                                          mc_position=mc_position.Mag(),
                                          mc_position_x=mc_position.X(),
                                          mc_position_y=mc_position.Y(),
                                          mc_position_z=mc_position.Z(),
                                          mc_momentum=mc_momentum.Mag(),
                                          mc_momentum_x=mc_momentum.X(),
                                          mc_momentum_y=mc_momentum.Y(),
                                          mc_momentum_z=mc_momentum.Z())

                    dedx_dict = dict(dedx=cluster_charge / path_length,
                                     dedx_with_mc=cluster_charge / mc_path_length,
                                     dedx_with_thickness=cluster_charge / cluster_thickness,
                                     p=mc_momentum.Mag(),
                                     perp_s_at_cluster_entry=perp_s_at_cluster_entry,
                                     perp_s_at_cluster_exit=perp_s_at_cluster_exit,
                                     track_charge=track_charge,
                                     path_length=path_length,
                                     mc_path_length=mc_path_length,
                                     p_origin=mc_particle.getMomentum().Mag())

                    track_dict = dict(track_helix_perigee_x=track_helix.getPerigeeX(),
                                      track_helix_perigee_y=track_helix.getPerigeeY(),
                                      track_helix_perigee_z=track_helix.getPerigeeZ(),
                                      track_helix_momentum_x=track_helix.getMomentumX(b_field),
                                      track_helix_momentum_y=track_helix.getMomentumY(b_field),
                                      track_helix_momentum_z=track_helix.getMomentumZ(b_field))

                    result_dict = dict()
                    result_dict.update(cluster_dict)
                    result_dict.update(mc_at_hit_dict)
                    result_dict.update(dedx_dict)
                    result_dict.update(track_dict)

            yield result_dict

    #: Refiners to be executed at the end of the harvesting / termination of the module
    #: Save a tree of all collected variables in a sub folder
    save_tree = refiners.SaveTreeRefiner()


class PXDHarvester(VXDHarvester):
    """Collect dE/dX and the correct p of each PXD hit of the MC particles. """

    def __init__(self, output_file_name, use_mc_info):
        """Constructor"""
        VXDHarvester.__init__(self, clusters="PXDClusters", detector=Belle2.Const.PXD, output_file_name=output_file_name,
                              use_mc_info=use_mc_info)


class SVDHarvester(VXDHarvester):
    """Collect dE/dX and the correct p of each SVD hit of the MC particles. """

    def __init__(self, output_file_name, use_mc_info):
        """Constructor"""
        VXDHarvester.__init__(self, clusters="SVDClusters", detector=Belle2.Const.SVD, output_file_name=output_file_name,
                              use_mc_info=use_mc_info)


class FitHarvester(QueueHarvester):
    """Collect dE/dX and the correct p of each VXD hit associated with the fitted tracks. """

    def __init__(self, output_file_name, queue):
        """Constructor"""
        QueueHarvester.__init__(self, queue, foreach="TrackFitResults", output_file_name=output_file_name)
        #: access the DataStore singletion
        self.data_store = Belle2.DataStore.Instance()

    def pick(self, track_fit_result):
        """Select a TrackFitResult if it is associated with exactly one MCParticle"""
        mc_track_cands = track_fit_result.getRelationsFrom("TrackCands")
        if len(mc_track_cands) != 1:
            return False

        mc_track_cand = mc_track_cands[0]
        mc_particles = self.data_store.getRelationsFromObj(mc_track_cand, "MCParticles")

        return len(mc_particles) == 1

    def peel(self, track_fit_result):
        """Aggregate the track-fit information associated with a TrackFitResult"""
        mc_track_cand = track_fit_result.getRelationsFrom("TrackCands")[0]
        mc_particle = self.data_store.getRelated(mc_track_cand, "MCParticles")

        fit_momentum = track_fit_result.getMomentum()
        true_momentum = mc_particle.getMomentum()

        related_reco_track = track_fit_result.getRelated("GF2Tracks")
        cardinal_rep = related_reco_track.getCardinalRep()
        kalman_fit_state = related_reco_track.getKalmanFitStatus()

        number_of_measurements_in_total = 0
        number_of_measurements_with_smaller_weight = 0

        number_of_momentum_measurements_in_total = 0
        number_of_momentum_measurements_with_smaller_weight = 0

        for track_point_ID in range(related_reco_track.getNumPointsWithMeasurement()):
            track_point = related_reco_track.getPointWithMeasurement(track_point_ID)

            is_momentum_measurement = track_point.getRawMeasurement().__class__.__name__ == "genfit::PlanarMomentumMeasurement"

            if is_momentum_measurement:
                number_of_momentum_measurements_in_total += 1

            if track_point.hasFitterInfo(cardinal_rep):
                fitter_info = track_point.getFitterInfo(cardinal_rep)
                num_measurements = fitter_info.getNumMeasurements()

                for measurement_id in range(num_measurements):
                    number_of_measurements_in_total += 1
                    weight = fitter_info.getMeasurementOnPlane(measurement_id).getWeight()
                    if weight != 1:
                        number_of_measurements_with_smaller_weight += 1

                        if is_momentum_measurement:
                            number_of_momentum_measurements_with_smaller_weight += 1

        return dict(fit_momentum_x=fit_momentum.X(),
                    fit_momentum_y=fit_momentum.Y(),
                    fit_momentum_z=fit_momentum.Z(),
                    p_value=kalman_fit_state.getForwardPVal(),
                    backward_p_value=kalman_fit_state.getBackwardPVal(),
                    true_momentum_x=true_momentum.X(),
                    true_momentum_y=true_momentum.Y(),
                    true_momentum_z=true_momentum.Z(),
                    number_of_measurements_in_total=number_of_measurements_in_total,
                    number_of_measurements_with_smaller_weight=number_of_measurements_with_smaller_weight,
                    number_of_momentum_measurements_in_total=number_of_momentum_measurements_in_total,
                    number_of_momentum_measurements_with_smaller_weight=number_of_momentum_measurements_with_smaller_weight)

    #: Refiners to be executed at the end of the harvesting / termination of the module
    #: Save a tree of all collected variables in a sub folder
    save_tree = refiners.SaveTreeRefiner()
