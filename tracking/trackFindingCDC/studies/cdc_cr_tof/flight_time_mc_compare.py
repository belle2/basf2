import basf2
from ROOT import Belle2
from ROOT import gSystem
from generators import add_cosmics_generator

gSystem.Load('libcdc')
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

import tracking
from tracking.harvest.harvesting import harvest
from tracking.harvest.refiners import (
    save_tree,
    save_pull_analysis,
    save_profiles,
    save_scatters,
)

DriftTimeUtil = Belle2.TrackFindingCDC.DriftTimeUtil


def main():
    path = basf2.create_path()

    # Create Event information
    path.add_module('EventInfoSetter',
                    evtNumList=[50000],
                    runList=[1],
                    )

    # Show progress of processing
    path.add_module('Progress')

    # Load gearbox parameters
    gearbox_module = path.add_module('Gearbox')

    # Create geometry
    geometry_module = path.add_module('Geometry')

    # Adjust gemetry for the test beam #
    # ################################ #
    cdc_top_geometry = False
    if cdc_top_geometry:
        gearbox_module.param(dict(fileName="geometry/CDCcosmicTests.xml"))  # <- does something mysterious to the reconstruction...
        # The peculiar behaviour seems to originate from the low magnetic field setup close to the earth magentic field.
        # In the current state of the reconstruction it seems to be better to set the magnetic field to zero
        # Override of the magnetic field is made below as a temporary work around until we discussed the topic with
        # the TOP group how composed the geometry/CDCcosmicTests.xml.
        gearbox_module.param(dict(overrideMultiple=[
            # Explicitly override magnetic field to zero instead of earth magnetic field as defined in CDC TOP geometry
            ("/DetectorComponent[@name='MagneticField']//Component/Z", "0", ""),
        ]
        ))

    else:
        # Use the regular geometry but only the cdc, no magnetic field.
        geometry_module.param(dict(components=["CDC"],))

    gearbox_module.param(dict(
        override=[
            # Reset the top volume: must be larger than the generated surface and higher than the detector
            # It is the users responsibility to ensure a full angular coverage
            ("/Global/length", "8.", "m"),
            ("/Global/width", "8.", "m"),
            ("/Global/height", "1.5", "m"),

            # Adjustments of the CDC setup
            ("/DetectorComponent[@name='CDC']//t0FileName", "t0.dat", ""),
            ("/DetectorComponent[@name='CDC']//xtFileName", "xt_noB_v1.dat", ""),
            # ("/DetectorComponent[@name='CDC']//bwFileName", "badwire_CDCTOP.dat", ""),
            ("/DetectorComponent[@name='CDC']//GlobalPhiRotation", "1.875", "deg"),
        ]
    ))

    # Generator setup #
    # ############### #
    # generator_module = "cry"
    generator_module = "cosmics"
    # generator_module = "particle_gun"

    if generator_module == "cry":
        add_cosmics_generator(path, accept_box=[0.5, 0.5, 0.5], keep_box=[0.5, 0.5, 0.5])

    elif generator_module == "cosmics":
        path.add_module("Cosmics",
                        # Require particles close to the counter
                        ipRequirement=1,
                        ipdr=5,
                        ipdz=15,
                        )

    elif generator_module == "particle_gun":
        # Generate muons starting in the IP
        # Yields outgoing arm only

        # PDG code of a muon
        muon_pdg_code = 13
        sector = 0
        phiBounds = [240 + 60.0 * sector % 360.0, 300 + 60.0 * sector % 360.0]

        path.add_module("ParticleGun",
                        pdgCodes=[muon_pdg_code, -muon_pdg_code],
                        nTracks=1,
                        varyNTracks=False,
                        momentumGeneration='uniform',
                        momentumParams=[2.0, 4.0],
                        phiGeneration='uniform',
                        phiParams=phiBounds,
                        thetaGeneration='uniform',
                        thetaParams=[70., 110.])

    else:
        raise ValueError("Unknown generator module key " + generator_module)

    # Flight time versus trigger setup #
    # ################################ #

    # Two alternative tof setups:
    #  *  Relative to plane sets the Monte Carlo time of flight
    #     such that zero is on first intersection with the trigger counter plane
    #  *  Relative to point sets the Monte Carlo time of flight
    #     such that zero is at the perigee relative to central point of the trigger counter plane
    use_tof_relative_to_trigger_plane = True
    use_tof_relative_to_trigger_point = not use_tof_relative_to_trigger_plane
    if use_tof_relative_to_trigger_plane:
        tof_mode = 1
    elif use_tof_relative_to_trigger_point:
        tof_mode = 2

    # Also counter the adjustments the CRY generator makes to the production time in case it is used.
    use_cry_generator = generator_module == "cry"

    path.add_module('CDCCosmicSelector',
                    lOfCounter=30.,
                    wOfCounter=10.,
                    xOfCounter=0.3744,
                    yOfCounter=0.,
                    zOfCounter=-1.284,

                    # xOfCounter = -0.6,
                    # yOfCounter = 5,
                    # zOfCounter = 16,

                    TOF=tof_mode,
                    cryGenerator=use_cry_generator,
                    )

    # Run simulation
    path.add_module('FullSim',
                    ProductionCut=1000000.
                    )

    path.add_module('CDCDigitizer',
                    # **{'2015AprRun' : True}  # <- only hits in superlayer4 are digitized
                    # TrigTimeJitter=8.,       # <- simulate trigger timing jitter
                    AddTimeOfFlight=True,
                    AddInWirePropagationDelay=True,
                    CorrectForWireSag=True,
                    )

    tracking.add_cdc_cr_track_finding(path,
                                      trigger_point=(0.3744, 0, -1.284),
                                      )

    # path.add_module("TFCDC_TrackFinderCosmics")
    # tracking.add_cdc_track_finding(path)
    # path.add_module("TrackFinderMCTruthRecoTracks")

    if use_tof_relative_to_trigger_plane:
        path.add_module("PlaneTriggerTrackTimeEstimator",
                        triggerPlanePosition=[0.3744, 0, -1.284],
                        triggerPlaneDirection=[0, 1, 0],
                        useFittedInformation=False)

    path.add_module("SetupGenfitExtrapolation")
    path.add_module("DAFRecoFitter")

    # Create a harvesting module for the time of flight
    @save_pull_analysis(part_name="time", aux_names=["wire_idx", ], groupby=["layer_sidx"], outlier_z_score=3,)
    @save_pull_analysis(part_name="time", aux_names=["layer_sidx", "time_truth", ], outlier_z_score=3,)
    @save_pull_analysis(part_name="drift_length", aux_names=["layer_sidx", ], outlier_z_score=3,)
    @save_tree(name="tree")
    @harvest(foreach="RecoTracks", show_results=True)
    def harvest_cdc_cr(reco_track):
        fit_status = reco_track.getTrackFitStatus()
        if not fit_status or not fit_status.isFitConverged():
            return

        track_points = reco_track.getHitPointsWithMeasurement()
        for track_point in track_points:
            kalman_fitter_info = track_point.getKalmanFitterInfo()
            try:
                measurement_on_plane = kalman_fitter_info.getFittedState()
            except:
                # Sometimes a throw happens from Genfit, skip track point
                continue

            abs_measurement = track_point.getRawMeasurement(0)
            daf_weight = kalman_fitter_info.getWeights().at(0)
            cdc_hit = abs_measurement.getCDCHit()
            wire_id = Belle2.WireID(cdc_hit.getID())
            layer_cidx = wire_id.getICLayer()
            wire_idx = wire_id.getIWire()
            cdc_sim_hit = cdc_hit.getRelated("CDCSimHits")

            fit_data = get_fit_data(measurement_on_plane)

            flight_time = fit_data["flight_time_estimate"]
            drift_time = DriftTimeUtil.getDriftTime(fit_data["drift_length_estimate"],
                                                    layer_cidx,
                                                    fit_data["rl"],
                                                    fit_data["alpha_estimate"],
                                                    fit_data["theta_estimate"],
                                                    )

            prop_time = DriftTimeUtil.getPropTime(wire_id, fit_data["z_estimate"])
            time_walk = DriftTimeUtil.getTimeWalk(wire_id, cdc_hit.getADCCount())

            reco_time = flight_time + drift_time + prop_time + time_walk
            smear = True
            measured_time = DriftTimeUtil.getMeasuredTime(wire_id, cdc_hit.getTDCCount(), smear)

            # MC Information
            flight_time_truth = cdc_sim_hit.getFlightTime()
            drift_length_truth = cdc_sim_hit.getDriftLength()
            incoming_arm = -1 if flight_time_truth < 0 else 1

            if drift_time > 1000:
                # There seems to be a bug in the CDCGeometryPar::getDriftTime function
                continue
            if drift_length_truth < 0.1 or drift_length_truth > 0.5:
                continue
            if daf_weight < 0.5:
                continue

            yield dict(drift_length_estimate=fit_data["drift_length_estimate"],
                       drift_length_variance=fit_data["drift_length_variance"],
                       drift_length_truth=drift_length_truth,
                       time_truth=reco_time,
                       time_estimate=measured_time,
                       time_delta=reco_time - measured_time,

                       flight_time_estimate=flight_time,
                       drift_time_estimate=drift_time,
                       prop_time_estimate=prop_time,
                       time_walk_estimate=time_walk,

                       rl_estimate=fit_data["rl"],
                       alpha_estimate=fit_data["alpha_estimate"],
                       theta_estimate=fit_data["theta_estimate"],
                       daf_weight=daf_weight,
                       flight_time_truth=flight_time_truth,
                       incoming_arm=incoming_arm,
                       layer_sidx=incoming_arm * layer_cidx,
                       wire_idx=wire_idx,
                       tdc=cdc_hit.getTDCCount(),
                       )

    path.add_module(harvest_cdc_cr)

    # Create a harvesting module for the time of flight mc comparision
    @save_pull_analysis(part_name="flight_time", groupby=[None, "incoming_arm"], outlier_z_score=5,)
    @save_tree(name="tree")
    @harvest(foreach="RecoTracks", show_results=True)
    def harvest_flight_times(reco_track):
        track_points = reco_track.getHitPointsWithMeasurement()
        for track_point in track_points:
            kalman_fitter_info = track_point.getKalmanFitterInfo()
            try:
                measurement_on_plane = kalman_fitter_info.getFittedState()
            except:
                # Sometimes a throw happens from Genfit, skip track point
                continue
            abs_measurement = track_point.getRawMeasurement(0)
            cdc_hit = abs_measurement.getCDCHit()
            cdc_sim_hit = cdc_hit.getRelated("CDCSimHits")

            flight_time_estimate = measurement_on_plane.getTime()
            flight_time_truth = cdc_sim_hit.getFlightTime()
            incoming_arm = flight_time_truth < 0

            yield dict(flight_time_estimate=flight_time_estimate,
                       flight_time_truth=flight_time_truth,
                       incoming_arm=incoming_arm)

    # path.add_module(harvest_flight_times)

    # generate events
    basf2.print_path(path)
    basf2.process(path)

    # show call statistics
    print(basf2.statistics)


# Helper functions #
# ################ #
def get_fit_data(measurement_on_plane):
    flight_time = measurement_on_plane.getTime()
    pos = measurement_on_plane.getPos()
    mom = measurement_on_plane.getMom()
    wirepos = measurement_on_plane.getPlane().getO()
    alpha = wirepos.DeltaPhi(mom)
    theta = mom.Theta()

    state_on_plane = measurement_on_plane.getState()  # TVectorD
    cov_on_plane = measurement_on_plane.getCov()     # TMatrixDSym

    # The state at index 3 is equivalent to the drift length
    drift_length = abs(state_on_plane(3))
    drift_length_var = abs(cov_on_plane(3, 3))
    rl_sign = 1 if state_on_plane(3) > 0 else -1
    rl = 1 if state_on_plane(3) > 0 else 0

    return dict(flight_time_estimate=flight_time,
                x_estimate=pos.X(),
                y_estimate=pos.Y(),
                z_estimate=pos.Z(),
                px_estimate=mom.X(),
                py_estimate=mom.Y(),
                pz_estimate=mom.Z(),
                wire_x_estimate=wirepos.X(),
                wire_y_estimate=wirepos.Y(),
                wire_z_estimate=wirepos.Z(),
                alpha_estimate=alpha,
                theta_estimate=theta,
                drift_length_estimate=drift_length,
                drift_length_variance=drift_length_var,
                rl=rl,
                rl_sign=rl_sign,
                )


if __name__ == "__main__":
    main()
