from basf2 import *

# Set the log level to show only warning, error and, fatal messages
# otherwise there's gonna be a segfault when python exits
set_log_level(LogLevel.WARNING)

# Load parameters from xml
gearbox = register_module('Gearbox')
# VXD (no Telescopes), and the real PCMAG magnetic field
gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB_v1.xml')

# Create geometry
geometry = register_module('Geometry')
# No magnetic field for this test,
geometry.param('components', ['TB'])

#input
input = register_module('SeqRootInput')
input.param('inputFileName', '../vxdtb/data/e0000r000391.sroot')
# data reader
dataMerger = register_module('TelDataMerger')
# use corresponding telescope data file
dataMerger.param('inputFileName', '../vxdtb/data/run000070.raw')
dataMerger.set_log_level(LogLevel.INFO)

progress = register_module('Progress')
dataWriter = register_module('RootOutput')
dataWriter.param('outputFileName', 'TelMergerOutput.root')

main = create_path()
main.add_module(input)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(dataMerger)
main.add_module(dataWriter)

process(main)
