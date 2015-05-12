import basf2

import trackfindingcdc.cdcdisplay as cdcdisplay

# Compose basf2 module path #
#############################
main_path = basf2.create_path()

# Master module: RootInput
root_input_module = basf2.register_module('RootInput')
root_input_module.param({'inputFileName': "cr_sample.root"})
main_path.add_module(root_input_module)

# gearbox & geometry needs to be registered any way
gearbox_module = basf2.register_module('Gearbox')
main_path.add_module(gearbox_module)

components = ["CDC"]
geometry_module = basf2.register_module('Geometry')
geometry_module.param('components', components)
main_path.add_module(geometry_module)

# Progress module
progress_module = basf2.register_module('Progress')
main_path.add_module(progress_module)


finder_module = basf2.register_module('TrackFinderCDCCosmics')
finder_module.param({"MinimalHitsForSingleSegmentTrackBySuperLayerId": {4: 1},
                     "FitSegments": True})
main_path.add_module(finder_module)

# cdc_display_module = cdcdisplay.CDCSVGDisplayModule("./")

# cdc_display_module.draw_gftrackcands = True
# cdc_display_module.draw_gftrackcand_trajectories = True
# main_path.add_module(cdc_display_module)

output = basf2.register_module('RootOutput')
output.param('outputFileName', 'cdc_cosmics_output.root')

main_path.add_module(output)

basf2.process(main_path)
