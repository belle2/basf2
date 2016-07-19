import basf2
import ROOT
import datetime
import os
from basf2 import *
from tracking import add_cdc_track_finding
from ROOT import Belle2
# reset_database()
# use_local_database()

# import trackfindingcdc.cdcdisplay as cdcdisplay
d = datetime.datetime.today()
print(d.strftime('This Calculution is done at : %d-%m-%y %H:%M:%S\n'))
# Compose basf2 module path #
#############################
main_path = basf2.create_path()


# Master module: RootInput
root_input_module = basf2.register_module('RootInput')
# root_input_module.param({'inputFileNames': [inputfilename]})
main_path.add_module(root_input_module)

# gearbox & geometry needs to be registered any way
gearbox_module = basf2.register_module('Gearbox')
gearbox_module.param('override', [
    ("/DetectorComponent[@name='CDC']//t04ReconFileName", "t0.dat", ""),
    ("/DetectorComponent[@name='CDC']//XtFileFormat", "1", ""),
    ("/DetectorComponent[@name='CDC']//xt4ReconFileName", "xt_noB_v1.dat", ""),
    ("/DetectorComponent[@name='CDC']//sigma4ReconFileName", "sigma_V0.1.dat", ""),
    ("/DetectorComponent[@name='CDC']//GlobalPhiRotation", "1.875", "deg"),
])
main_path.add_module(gearbox_module)

geometry_module = basf2.register_module('Geometry',
                                        components=['CDC'])
main_path.add_module(geometry_module)

# Progress module
progress_module = basf2.register_module('Progress')
main_path.add_module(progress_module)

# track finding (Legendre finder)
add_cdc_track_finding(main_path)
# main_path.add_module("TrackFinderCDCCosmics",
#                     flightTimeEstimation = "downwards"
#                     )

extrap = basf2.register_module("SetupGenfitExtrapolation")
main_path.add_module(extrap)

main_path.add_module("PlaneTriggerTrackTimeEstimator",
                     triggerPlanePosition=[0, 0, 0],
                     triggerPlaneDirection=[0, 1, 0],
                     useFittedInformation=False)

main_path.add_module("DAFRecoFitter",
                     probCut=0.00001,
                     pdgCodesToUseForFitting=13
                     )
main_path.add_module('TrackCreator')
results = basf2.register_module('CDCCRTest',
                                RecoTracksColName='RecoTracks',
                                histogramDirectoryName='trackfit',
                                Output='xt.root',
                                fillExpertHistograms=False,
                                plotResidual=False,
                                calExpectedDriftTime=True,
                                CorrectToF=True,
                                CorrectToP=True,
                                TriggerPos=[0.3774, 0, -1.284],
                                IwireLow=[45, 45, 60, 60, 75, 95, 95, 110, 125],
                                #                                IwireUpper = [65, 65, 80, 100, 115, 130, 130, 145, 160]
                                #                                IwireLow = [115, 125, 155, 160 205, 235, 255, 285, 315],
                                IwireUpper=[165, 145, 180, 210, 242, 260, 290, 325, 355]
                                #                                m_hitEfficiency = False,
                                )
results.logging.log_level = basf2.LogLevel.INFO
main_path.add_module(results)

histman = basf2.register_module('HistoManager',
                                histoFileName='output.root')
main_path.add_module(histman)

# output = basf2.register_module('RootOutput')
# output.param('outputFileName', 'cdc_tracking.root')
# main_path.add_module(output)


# For some reason the Display cannot show the fitted tracks ...
# display = basf2.register_module("Display")
# main_path.add_module(display)

# ... so you can use the GenfitVis module instead.  This only shows
# the tracks at the end of the job, so be sure to process only a few
# events if you want to use it.
#
# genfitVis = basf2.register_module("GenfitVis")
# main_path.add_module(genfitVis)

basf2.process(main_path)
d = datetime.datetime.today()
print(d.strftime('Finish at : %y-%m-%d %H:%M:%S\n'))
print(statistics)
