from tracking.validation.harvesting import HarvestingModule
from tracking.run.event_generation import MinimalRun
from tracking.validation import refiners
from ROOT import Belle2
import numpy as np
import ROOT
from tracking.validation.pr_side_module import PRSideTrackingValidationModule
from tracking.validation.module import TrackingValidationModule

from tracking.ipython_tools import QueueHarvester


class VXDMomentumEnergyEstimator:

    """ The base class with all static methods to use. """

    @staticmethod
    def do_for_each_hit_type(cluster, svd_function, pxd_function):
        cluster_type = cluster.__class__.__name__
        if cluster_type == "Belle2::PXDCluster":
            return pxd_function(cluster)
        elif cluster_type == "Belle2::SVDCluster":
            return svd_function(cluster)
        else:
            raise TypeError("Unknown hit type")

    @staticmethod
    def calculate_charges_and_path_lengths_for_one_type(clusters, mc_particle):
        charge_list = []
        path_length_list = []

        tools = Belle2.VXDMomentumEstimationTools(clusters[0].__class__.__name__).getInstance()

        momentum = mc_particle.getMomentum()
        position = mc_particle.getProductionVertex()
        charge = mc_particle.getCharge()
        helix = Belle2.Helix(position, momentum, charge, 1.5)

        for cluster in clusters:

            charge = tools.getCalibratedCharge(cluster)
            path_length = tools.getPathLength(cluster, helix)

            charge_list.append(calibrated_charge)
            path_length_list.append(path_length)

        return charge_list, path_length_list, list(np.divide(charge_list, path_length_list))

    @staticmethod
    def generate_truncated(charge_list):
        sorted_list = sorted(charge_list)
        if len(sorted_list) > 2:
            return sorted_list[:-2], sorted_list[:4], sorted_list[:6]
        else:
            return sorted_list, sorted_list[:4], sorted_list[:6]


class MCTrajectoryHarvester(HarvestingModule):

    """ A harvester to check for the positions of the track points in the MCParticleTrajectories"""

    def __init__(self):
        HarvestingModule.__init__(self, foreach="MCParticleTrajectorys", output_file_name="mc_trajectory.root")

    def peel(self, mc_particle_trajectory):
        for track_point in mc_particle_trajectory:
            yield {"x": track_point.x, "y": track_point.y, "z": track_point.z, "index": self.counter}

    save_tree = refiners.SaveTreeRefiner()


class MCParticleHarvester(HarvestingModule):

    """ A harvester to redo parts of the analysis in the Belle II Paper by Robert """

    def __init__(self):
        HarvestingModule.__init__(self, foreach="MCParticles", output_file_name="mc_particle.root")

    def pick(self, mc_particle):
        pxd_clusters = mc_particle.getRelationsFrom("PXDClusters")
        svd_clusters = mc_particle.getRelationsFrom("PXDClusters")
        return (mc_particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle) and
                abs(mc_particle.getPDG()) == 211 and
                len(pxd_clusters) + len(svd_clusters) > 0)

    def generate_cluster_dicts(self, charge_list, path_length_list, normalized_charge_list, name):
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

    save_tree = refiners.SaveTreeRefiner()


class VXDHarvester(QueueHarvester):

    """ A base class for the VXD hitwise analysis. Collect dE/dX and the correct p of each hit of the MC particles. """

    def __init__(self, foreach, output_file_name):
        HarvestingModule.__init__(self, foreach=foreach, output_file_name=output_file_name)

        self.svd_tools = Belle2.VXDMomentumEstimationTools("Belle2::SVDCluster").getInstance()
        self.pxd_tools = Belle2.VXDMomentumEstimationTools("Belle2::PXDCluster").getInstance()

    def pick(self, cluster):
        mc_particles = cluster.getRelationsTo("MCParticles")

        space_point = cluster.getRelated("SpacePoints")

        if space_point is None:
            return False

        for mc_particle in mc_particles:
            if (mc_particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle) and
                    abs(mc_particle.getPDG()) == 211):

                return True

        return False

    def peel(self, cluster):
        mc_particles = cluster.getRelationsTo("MCParticles")

        for mc_particle in mc_particles:
            if (mc_particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle) and abs(mc_particle.getPDG()) == 211):

                momentum = mc_particle.getMomentum()
                position = mc_particle.getProductionVertex()
                charge = mc_particle.getCharge()
                helix = Belle2.Helix(position, momentum, int(charge), 1.5)

                charge = VXDMomentumEnergyEstimator.do_for_each_hit_type(
                    cluster,
                    lambda cluster: self.svd_tools.getCalibratedCharge(cluster),
                    lambda cluster: self.pxd_tools.getCalibratedCharge(cluster))
                path_length = VXDMomentumEnergyEstimator.do_for_each_hit_type(
                    cluster, lambda cluster: self.svd_tools.getPathLength(
                        cluster, helix), lambda cluster: self.pxd_tools.getPathLength(
                        cluster, helix))

                mc_momentum = VXDMomentumEnergyEstimator.do_for_each_hit_type(
                    cluster,
                    lambda cluster: self.svd_tools.getMomentumOfMCParticle(cluster),
                    lambda cluster: self.pxd_tools.getMomentumOfMCParticle(cluster))

                p = mc_momentum.Mag()

                is_u = VXDMomentumEnergyEstimator.do_for_each_hit_type(
                    cluster,
                    lambda cluster: cluster.isUCluster(),
                    lambda cluster: np.NaN)
                is_pxd = VXDMomentumEnergyEstimator.do_for_each_hit_type(cluster, lambda cluster: False, lambda cluster: True)

        return dict(charge=charge,
                    p=p,
                    path_length=path_length,
                    is_u=is_u,
                    p_origin=mc_particle.getMomentum().Mag(),
                    is_pxd=is_pxd)

    save_tree = refiners.SaveTreeRefiner()


class PXDHarvester(VXDHarvester):

    def __init__(self, output_file_name):
        VXDHarvester.__init__(self, foreach="PXDClusters", output_file_name=output_file_name)


class SVDHarvester(VXDHarvester):

    def __init__(self, output_file_name):
        VXDHarvester.__init__(self, foreach="SVDClusters", output_file_name=output_file_name)


class FitHarvester(QueueHarvester):

    def __init__(self, output_file_name, queue):
        QueueHarvester.__init__(self, queue, foreach="TrackFitResults", output_file_name=output_file_name)
        self.data_store = Belle2.DataStore.Instance()

    def pick(self, track_fit_result):
        mc_track_cands = track_fit_result.getRelationsFrom("TrackCands")
        if len(mc_track_cands) != 1:
            return False

        mc_track_cand = mc_track_cands[0]
        mc_particles = self.data_store.getRelationsFromObj(mc_track_cand, "MCParticles")

        return len(mc_particles) == 1

    def peel(self, track_fit_result):
        mc_track_cand = track_fit_result.getRelationsFrom("TrackCands")[0]
        mc_particle = self.data_store.getRelated(mc_track_cand, "MCParticles")

        fit_momentum = track_fit_result.getMomentum()
        true_momentum = mc_particle.getMomentum()

        related_reco_track = track_fit_result.getRelated("RecoTracks")
        cardinal_rep = related_reco_track.getCardinalRep()

        number_of_measurements_in_total = 0
        number_of_measurements_with_smaller_weight = 0

        number_of_momentum_measurements_in_total = 0
        number_of_momentum_measurements_with_smaller_weight = 0

        for track_point_ID in xrange(related_reco_track.getNumPointsWithMeasurement()):
            track_point = related_reco_track.getPointWithMeasurement(track_point_ID)

            is_momentum_measurement = track_point.getRawMeasurement().__class__.__name__ == "genfit::PlanarMomentumMeasurement"

            if is_momentum_measurement:
                number_of_momentum_measurements_in_total += 1

            if track_point.hasFitterInfo(cardinal_rep):
                fitter_info = track_point.getFitterInfo(cardinal_rep)
                num_measurements = fitter_info.getNumMeasurements()

                for measurement_id in xrange(num_measurements):
                    number_of_measurements_in_total += 1
                    weight = fitter_info.getMeasurementOnPlane(measurement_id).getWeight()
                    if weight != 1:
                        number_of_measurements_with_smaller_weight += 1

                        if is_momentum_measurement:
                            number_of_momentum_measurements_with_smaller_weight += 1

        return dict(fit_momentum_x=fit_momentum.X(),
                    fit_momentum_y=fit_momentum.Y(),
                    fit_momentum_z=fit_momentum.Z(),
                    p_value=track_fit_result.getPValue(),
                    true_momentum_x=true_momentum.X(),
                    true_momentum_y=true_momentum.Y(),
                    true_momentum_z=true_momentum.Z(),
                    number_of_measurements_in_total=number_of_measurements_in_total,
                    number_of_measurements_with_smaller_weight=number_of_measurements_with_smaller_weight,
                    number_of_momentum_measurements_in_total=number_of_momentum_measurements_in_total,
                    number_of_momentum_measurements_with_smaller_weight=number_of_momentum_measurements_with_smaller_weight)

    save_tree = refiners.SaveTreeRefiner()
