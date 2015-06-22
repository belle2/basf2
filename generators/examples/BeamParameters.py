from basf2 import *
from beamparameters import add_beamparameters

# small example to show how to set BeamParameters

main = create_path()
main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=10000)
# add beamparameters with named preset
beamparameters = add_beamparameters(main, "Y4S")
# override preset however you like
beamparameters.param("covHER", [0.01, 0.01, 0.01])
beamparameters.param("smearDirection", False)
# print parameters to be used
print_params(beamparameters)
# add generator, simulation, analysis, and so forth
main.add_module("RootOutput", outputFileName="BeamParameters.root")
process(main)
