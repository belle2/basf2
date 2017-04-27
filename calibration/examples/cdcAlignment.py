from basf2 import *
set_log_level(LogLevel.INFO)

import ROOT
from ROOT import Belle2

from caf.framework import Calibration, CAF
from caf import backends


def createMillepedeCDCLayerAlignment(input_files=[], magnet=True, cosmics=True, name="CDCAlignment", p_value_cut=0):
    # Millepede Collector Setup
    col_millepede = register_module('MillepedeCollector', components=['CDCAlignment'], minPValue=p_value_cut)
    # col_millepede.param('granularity', 'all')  # Allows us to execute algorithm over all data, in one big IoV

    # Create a path to run before the collector, can optionally include RootInput here to configure it,
    # but the inputfiles will be setup by the CAF.
    # Copied in collector setup from alignment/examples/3_collect.py with NO Magnetic field for cosmic events
    collector_pre_path = create_path()
    collector_pre_path.add_module('RootInput')
    collector_pre_path.add_module('Progress')
    collector_pre_path.add_module('Gearbox')
    if magnet:
        collector_pre_path.add_module('Geometry')
    else:
        collector_pre_path.add_module('Geometry', excludedComponents=['MagneticField'])

    collector_pre_path.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)

    components = ROOT.vector('string')()
    components.push_back('CDCAlignment')
    # MillepedeAlgorithm Setup
    alg_millepede = ROOT.Belle2.MillepedeAlgorithm()
    alg_millepede.setComponents(components)
    alg_millepede.steering().command('method inversion 3 0.1')
    alg_millepede.steering().command('entries 10')
    alg_millepede.steering().command('hugecut 50')
    alg_millepede.steering().command('chiscut 30. 6.')
    alg_millepede.steering().command('outlierdownweighting 3')
    alg_millepede.steering().command('dwfractioncut 0.1')
    alg_millepede.steering().command('Parameters')

    def fixLayerParam(layer, param):
        label = Belle2.GlobalLabel()
        label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), layer, param)
        alg_millepede.steering().command(str(label.label()) + ' 0.0 -1.')

    fixLayerParam(0, 1)  # X
    fixLayerParam(0, 2)  # Y
    fixLayerParam(55, 1)  # X
    fixLayerParam(55, 2)  # Y

    def pre_alg_millepede(algorithm, iteration):
        input = register_module('RootInput', inputFileNames=input_files)
        input.initialize()
        pass
    # Setup the actual Calibration and CAF and run
    cal = Calibration(name, col_millepede, alg_millepede, input_files)
    cal.output_patterns.append('Belle2FileCatalog.xml')
    cal.output_patterns.append('*.mille')
    # Adding in setup basf2 paths and functions for the collector and algorithm
    cal.pre_collector_path = collector_pre_path
    cal.pre_algorithms = pre_alg_millepede

    return cal


def main(argv):

    # Create a CAF instance to configure how we will run
    fw = CAF()
    fw.max_iterations = 2
    inputFiles = [os.path.abspath('dst.root')]
    print(inputFiles)
    fw.add_calibration(createMillepedeCDCLayerAlignment(input_files=inputFiles))
    fw.output_dir = 'cdc_alignment'
    # Start her up!
    fw.run()
    print("Finished CAF Processing")

if __name__ == "__main__":
    main(sys.argv[1:])
