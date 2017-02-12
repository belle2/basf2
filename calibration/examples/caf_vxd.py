from basf2 import *
set_log_level(LogLevel.INFO)

import ROOT
from ROOT.Belle2 import TestCalibrationAlgorithm
from ROOT.Belle2 import PXDClusterShapeCalibrationAlgorithm

from caf.framework import Calibration, CAF
from caf import backends


def main(argv):
    if len(argv) == 1:
        data_dir = argv[0]
    else:
        print("Usage: basf2 CAF_multiple_options.py <data directory>")
        sys.exit(1)

    # Millepede Collector Setup
    col_millepede = register_module('MillepedeCollector')
#    col_millepede.param('granularity', 'all')  # Allows us to execute algorithm over all data, in one big IoV

    # Create a path to run before the collector, can optionally include RootInput here to configure it,
    # but the inputfiles will be setup by the CAF.
    # Copied in collector setup from alignment/examples/3_collect.py with NO Magnetic field for cosmic events
    vxd_collector_pre_path = create_path()
    vxd_collector_pre_path.add_module('RootInput', excludeBranchNames=['GF2TracksToMCParticles',
                                                                       'TrackCandsToGF2Tracks',
                                                                       'GF2Tracks'])
    vxd_collector_pre_path.add_module('Gearbox')
    vxd_collector_pre_path.add_module('Geometry', components=['BeamPipe', 'PXD', 'SVD'])

    vxd_collector_pre_path.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)
    vxd_collector_pre_path.add_module('GBLfit')

    # MillepedeAlgorithm Setup
    # Copied in algorithm setup from alignment/examples/5_calibrate.py
    alg_millepede = ROOT.Belle2.MillepedeAlgorithm()
    alg_millepede.steering().command('method diagonalization 1 0.1')
#    alg_millepede.steering().command('entries 100')
    alg_millepede.steering().command('hugecut 100000')
    alg_millepede.steering().command('chiscut 3000. 600.')
    alg_millepede.steering().command('outlierdownweighting 3')
    alg_millepede.steering().command('dwfractioncut 0.1')
    alg_millepede.steering().command('Parameters')
    for vxdid in ROOT.Belle2.VXD.GeoCache.getInstance().getListOfSensors():
        # Fix 6th SVD layer
        if vxdid.getLayerNumber() != 6:
            continue
        label = ROOT.Belle2.GlobalLabel(vxdid, 0)
        for ipar in range(1, 7):
            par_label = label.label() + ipar
            cmd = str(par_label) + ' 0.0 -1.'
            alg_millepede.steering().command(cmd)

    def pre_alg_millepede(algorithm, iteration):
        """
        MillepedeAlgorithm needs some geometry initialisation before it runs.
        """
        B2INFO("Running setup function")
        gear = register_module('Gearbox')
        gear.initialize()
        geom = register_module('Geometry')
        geom.param('components', ['PXD', 'SVD'])
        geom.initialize()

    # Setup the input data
    # If you're using data from calibration/examples/1_create_sample_DSTs.sh
    # then this won't be enough data to do an alignment and I expect the CAF
    # will fail.
    vxd_align_input_files = []
    for run in range(1, 5):
        cosmics_file_path = os.path.abspath(os.path.join(data_dir, 'DST_exp1_run{0}.root'.format(run)))
        vxd_align_input_files.append(cosmics_file_path)

    # Setup the actual Calibration and CAF and run
    cal_vxd_align = Calibration('VXD_alignment', col_millepede, alg_millepede, vxd_align_input_files)
    cal_vxd_align.output_patterns.append('Belle2FileCatalog.xml')
    cal_vxd_align.output_patterns.append('*.mille')
    # Adding in setup basf2 paths and functions for the collector and algorithm
    cal_vxd_align.pre_collector_path = vxd_collector_pre_path
    cal_vxd_align.pre_algorithms = pre_alg_millepede

    # Create a CAF instance to configure how we will run
    fw = CAF()
    fw.max_iterations = 5
    fw.add_calibration(cal_vxd_align)
    fw.output_dir = 'calibration_vxd'
    # Start her up!
    fw.run()
    print("Finished CAF Processing")

if __name__ == "__main__":
    main(sys.argv[1:])
