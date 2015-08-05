from tracking.validation.harvesting import HarvestingModule
from tracking.run.event_generation import MinimalRun
from tracking.validation import refiners
from ROOT import Belle2
import numpy as np
import ROOT


class VXDMomentumEnergyEstimator:
    calibration = 0.653382
    layer_positions = [1.42, 2.18, 3.81, 8.0, 10.51, 13.51]

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
    def get_thickness_of_cluster(cluster):
        vxdID = cluster.getSensorID()
        sensorInfoBase = Belle2.VXD.GeoCache.getInstance().getSensorInfo(vxdID)

        return sensorInfoBase.getThickness() / 100

    @staticmethod
    def get_radius_of_cluster(cluster):
        space_point = cluster.getRelated("SpacePoints")
        if not space_point:
            sensor_id = cluster.getSensorID()
            layer = sensor_id.getLayerNumber()
            radius = VXDMomentumEnergyEstimator.layer_positions[layer - 1]
        else:
            radius = space_point.getPosition().Perp()

        return radius / 100

    @staticmethod
    def calculate_charge_and_path_length(cluster, trajectory2D, trajectorySZ):
        Vector2D = Belle2.TrackFindingCDC.Vector2D

        def pxd_function(cluster):
            calibration = VXDMomentumEnergyEstimator.calibration
            return calibration

        def svd_function(cluster):
            calibration = 1
            return calibration

        calibration = VXDMomentumEnergyEstimator.do_for_each_hit_type(cluster, svd_function, pxd_function)

        thickness = VXDMomentumEnergyEstimator.get_thickness_of_cluster(cluster)

        charge = cluster.getCharge()
        radius = VXDMomentumEnergyEstimator.get_radius_of_cluster(cluster)

        layer_inner_position_on_x_axis = Vector2D(radius - thickness / 2.0, 0)
        layer_outer_position_on_x_axis = Vector2D(radius + thickness / 2.0, 0)

        cluster_layer_entry = trajectory2D.getCloseSameCylindricalR(layer_inner_position_on_x_axis)
        cluster_layer_exit = trajectory2D.getCloseSameCylindricalR(layer_outer_position_on_x_axis)

        perp_s_at_cluster_entry = trajectory2D.calcPerpS(cluster_layer_entry)
        perp_s_at_cluster_exit = trajectory2D.calcPerpS(cluster_layer_exit)

        z_at_cluster_entry = trajectorySZ.mapSToZ(perp_s_at_cluster_entry)
        z_at_cluster_exit = trajectorySZ.mapSToZ(perp_s_at_cluster_exit)

        distance_2D = trajectory2D.calcPerpSBetween(cluster_layer_entry, cluster_layer_exit)
        distance_Z = z_at_cluster_entry - z_at_cluster_exit
        distance_3D = np.sqrt(distance_2D ** 2 + distance_Z ** 2)

        return calibration * charge, distance_3D

    @staticmethod
    def calculate_charges_and_path_lengths_for_one_type(clusters, trajectory_of_particle):
        charge_list = []
        path_length_list = []

        trajectory2D = trajectory_of_particle.getTrajectory2D()
        trajectorySZ = trajectory_of_particle.getTrajectorySZ()

        for cluster in clusters:
            calibrated_charge, path_length = VXDMomentumEnergyEstimator.calculate_charge_and_path_length(
                cluster, trajectory2D, trajectorySZ)
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

    @staticmethod
    def calculate_trajectory(mc_particle, cluster):

        # We use the momentum at the origin. This is wrong - but later we will have about the same information!
        momentum = mc_particle.getMomentum()
        position = mc_particle.getProductionVertex()

        charge = mc_particle.getCharge()

        CDCTrajectory3D = Belle2.TrackFindingCDC.CDCTrajectory3D
        Vector3D = Belle2.TrackFindingCDC.Vector3D

        trajectory3D = CDCTrajectory3D(Vector3D(position), Vector3D(momentum), charge)

        return trajectory3D

    @staticmethod
    def get_momentum(cluster):
        def svd_function(cluster):
            true_hit = cluster.getRelated("SVDTrueHits")
            vxdID = cluster.getSensorID()
            sensorInfoBase = Belle2.VXD.GeoCache.getInstance().getSensorInfo(vxdID)
            momentum = sensorInfoBase.vectorToGlobal(true_hit.getMomentum())

            return momentum

        def pxd_function(cluster):
            true_hit = cluster.getRelated("PXDTrueHits")
            vxdID = cluster.getSensorID()
            sensorInfoBase = Belle2.VXD.GeoCache.getInstance().getSensorInfo(vxdID)
            momentum = sensorInfoBase.vectorToGlobal(true_hit.getMomentum())

            return momentum

        return VXDMomentumEnergyEstimator.do_for_each_hit_type(cluster, svd_function, pxd_function)


class MCTrajectoryHarvester(HarvestingModule):

    def __init__(self):
        HarvestingModule.__init__(self, foreach="MCParticleTrajectorys", output_file_name="mc_trajectory.root")

    def peel(self, mc_particle_trajectory):
        for track_point in mc_particle_trajectory:
            yield {"x": track_point.x, "y": track_point.y, "z": track_point.z, "index": self.counter}

    save_tree = refiners.SaveTreeRefiner()


class MCParticleHarvester(HarvestingModule):

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

        truncated, first4, first6 = self.generate_truncated(normalized_charge_list)

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

        trajectory3D = VXDMomentumEnergyEstimator.calculate_trajectory(mc_particle)

        pxd_results = self.calculate_charges_and_path_lengths_for_one_type(pxd_clusters, trajectory3D)
        svd_results = self.calculate_charges_and_path_lengths_for_one_type(svd_clusters, trajectory3D)

        pxd_cluster_dicts = self.generate_cluster_dicts(*pxd_results, name="pxd")
        pxd_charges, pxd_path_length, pxd_normalized_charges = pxd_results

        svd_charges, svd_path_length, svd_normalized_charges = svd_results
        svd_cluster_dicts = self.generate_cluster_dicts(*svd_results, name="svd")

        combined_cluster_dicts = self.generate_cluster_dicts(pxd_charges + svd_charges,
                                                             pxd_path_length + svd_path_length,
                                                             pxd_normalized_charges + svd_normalized_charges,
                                                             name="combined")

        result = dict(p=momentum.Mag(), theta=momentum.Theta())
        result.update(pxd_cluster_dicts)
        result.update(svd_cluster_dicts)
        result.update(combined_cluster_dicts)

        return result

    save_tree = refiners.SaveTreeRefiner()


class SVDHarvester(HarvestingModule):

    def __init__(self):
        HarvestingModule.__init__(self, foreach="SVDClusters", output_file_name="svd.root")

    def pick(self, svd_cluster):
        mc_particles = svd_cluster.getRelationsTo("MCParticles")

        for mc_particle in mc_particles:
            if (mc_particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle) and
                    abs(mc_particle.getPDG()) == 211):
                return True

        return False

    def peel(self, svd_cluster):
        mc_particles = svd_cluster.getRelationsTo("MCParticles")

        for mc_particle in mc_particles:
            if (mc_particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle) and abs(mc_particle.getPDG()) == 211):
                trajectory3D = VXDMomentumEnergyEstimator.calculate_trajectory(mc_particle, svd_cluster)
                trajectory2D = trajectory3D.getTrajectory2D()
                trajectorySZ = trajectory3D.getTrajectorySZ()

                charge, path_length = VXDMomentumEnergyEstimator.calculate_charge_and_path_length(
                    svd_cluster, trajectory2D, trajectorySZ)

                mc_momentum = VXDMomentumEnergyEstimator.get_momentum(svd_cluster)

                p = mc_momentum.Mag()
                is_u = svd_cluster.isUCluster()

        return dict(charge=charge,
                    p=p,
                    path_length=path_length,
                    is_u=is_u,
                    p_origin=mc_particle.getMomentum().Mag())

    save_tree = refiners.SaveTreeRefiner()
