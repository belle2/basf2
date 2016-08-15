import basf2
from ROOT import Belle2

import tracking
from tracking.harvest.harvesting import harvest
from tracking.harvest.refiners import save_tree, save_pull_analysis


def main():
    path = basf2.create_path()

    # Create Event information
    path.add_module('EventInfoSetter',
                    evtNumList=[5000],
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
    generator_module = "cry"
    # generator_module = "cosmics"
    # generator_module = "particle_gun"

    if generator_module == "cry":
        # Register the CRY generator
        path.add_module('CRYInput',
                        # cosmic data input
                        CosmicDataDir=Belle2.FileSystem.findFile('data/generators/modules/cryinput/'),
                        # CosmicDataDir='.'
                        # user input file
                        SetupFile='cry.setup',

                        # acceptance half-lengths - at least one particle has to enter that box to use that event
                        acceptLength=0.5,
                        acceptWidth=0.5,
                        acceptHeight=0.5,
                        maxTrials=10000,

                        # keep half-lengths - all particles that do not enter the box are removed (keep box >= accept box)
                        keepLength=0.5,
                        keepWidth=0.5,
                        keepHeight=0.5,

                        # minimal kinetic energy - all particles below that energy are ignored
                        kineticEnergyThreshold=0.01,
                        )

    elif generator_module == "cosmics":
        path.add_module("Cosmics")

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

    # path.add_module("TrackFinderCDCCosmics")
    # tracking.add_cdc_track_finding(path)
    # path.add_module("TrackFinderMCTruthRecoTracks")

    if use_tof_relative_to_trigger_plane:
        path.add_module("PlaneTriggerTrackTimeEstimator",
                        triggerPlanePosition=[0.3744, 0, -1.284],
                        triggerPlaneDirection=[0, 1, 0],
                        useFittedInformation=False)

    path.add_module("SetupGenfitExtrapolation")
    path.add_module("DAFRecoFitter")

    # Create a harvesting module
    @save_pull_analysis(part_name="time", groupby=[None, "incoming_arm"], outlier_z_score=5,)
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

            time_estimate = measurement_on_plane.getTime()

            abs_measurement = track_point.getRawMeasurement(0)
            cdc_hit = abs_measurement.getCDCHit()

            cdc_sim_hit = cdc_hit.getRelated("CDCSimHits")
            time_truth = cdc_sim_hit.getFlightTime()
            incoming_arm = time_truth < 0

            yield dict(time_estimate=time_estimate,
                       time_truth=time_truth,
                       incoming_arm=incoming_arm)

    path.add_module(harvest_flight_times)

    # generate events
    basf2.print_path(path)
    basf2.process(path)

    # show call statistics
    print(basf2.statistics)


if __name__ == "__main__":
    main()
