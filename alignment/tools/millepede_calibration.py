#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
set_log_level(LogLevel.INFO)

if __name__ == '__main__':
    import os
    import sys

    import ROOT
    from ROOT import Belle2

    reset_database()
    use_database_chain()
    use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
    use_local_database(Belle2.FileSystem.findFile("localdb/database.txt"))

    import argparse
    parser = argparse.ArgumentParser(
        description="Universal tool to run Millepede collection and calibration (outside CAF, but easily portable)")
    parser.add_argument('action', action='store', type=str, help='What to do? full (collect + calibrate) / collect / calibrate')
    parser.add_argument(
        'scenario',
        action='store',
        type=str,
        help='Name of the scenario: global | CDCLayerAlignment | ... ')
    parser.add_argument('--magnet-off', dest='magnet_off', action='store_const', const=True,
                        default=False, help='Special switch to collect sample with no magnetic field')

    args = parser.parse_args()

    magnet = not bool(args.magnet_off)

    from alignment import MillepedeCalibration

    millepede = None

    if args.scenario == 'global':
        # Default global config:
        millepede = MillepedeCalibration([], magnet=magnet)
        # For mis-alignment/mis-calibration done by using "wrong" constants during collection in MC
        millepede.algo.invertSign()
        millepede.algo.steering().command('Fortranfiles')
        millepede.algo.steering().command('constraints.txt')

    if args.scenario == 'BeamParameters':
        print('For the BeamParameters scenario you need ParticleList named "Z0:mumu" with')
        print('vertex+beam constrained decays (with updated daughters) in your input DST for collection')

        millepede = MillepedeCalibration(['BeamParameters'], primary_vertices=['Z0:mumu'], magnet=magnet)
        millepede.algo.invertSign()

    if args.scenario == 'VXDHalfShellsAlignment':
        millepede = MillepedeCalibration(['VXDAlignment'], magnet=magnet)
        millepede.algo.invertSign()
        millepede.fixSVDPat()
        for layer in range(1, 7):
            for ladder in range(1, 17):
                # Fix also all ladders
                millepede.fixVXDid(layer, ladder, 0)
                for sensor in range(1, 6):
                    # Fix all sensors
                    millepede.fixVXDid(layer, ladder, sensor)
                    pass

    if args.scenario == 'VXDAlignment':
        millepede = MillepedeCalibration(
            ['VXDAlignment'],
            magnet=magnet,
            primary_vertices=['Z0:mumu'],
            particles=['mu+:bbmu'],
            tracks=['CosmicRecoTracks'])
        millepede.algo.invertSign()
        # Add the constraints (auto-generated from hierarchy), so you can
        # play with unfixing degrees of freedom below
        # millepede.algo.steering().command('Fortranfiles')
        # millepede.algo.steering().command('constraints.txt')
        millepede.fixVXDid(1, 1, 1)

    if args.scenario == 'CDCLayerAlignment':
        millepede = MillepedeCalibration(['CDCAlignment', 'CDCLayerAlignment'], magnet=magnet)
        millepede.algo.invertSign()
        millepede.fixCDCLayerX(49)
        millepede.fixCDCLayerY(49)
        millepede.fixCDCLayerRot(49)
        millepede.fixCDCLayerX(55)
        millepede.fixCDCLayerY(55)
        millepede.fixCDCLayerRot(55)

    if args.scenario == 'CDCTimeWalks':
        millepede = MillepedeCalibration(['CDCTimeWalks'], magnet=magnet)
        millepede.algo.invertSign()
        millepede.fixCDCTimeWalk(1)

    if args.scenario == 'BKLMAlignment':
        millepede = MillepedeCalibration(['BKLMAlignment'], magnet=magnet)
        millepede.algo.invertSign()

    if args.scenario == 'EKLMAlignment':
        millepede = MillepedeCalibration(['EKLMAlignment'], magnet=magnet)
        millepede.algo.invertSign()

    if not millepede:
        print('You have to select some scenario, e.g. global')
        sys.exit(1)

    if args.action in ['full', 'collect']:
        millepede.collect()
    if args.action in ['full', 'calibrate']:
        millepede.calibrate()
