# Path building
from basf2 import create_path, register_module, LogLevel, process, statistics
from simulation import add_simulation
from tracking import add_prefilter_tracking_reconstruction

path = create_path()

path.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=10)

# generate BBbar events
path.add_module('EvtGenInput')

# detector simulation, don't perfrom PXD data reduction
add_simulation(path, bkgOverlay=False, forceSetPXDDataReduction=True, usePXDDataReduction=False, cleanupPXDDataReduction=False)

# add tracking and track fitting, without MC matching
add_prefilter_tracking_reconstruction(path)

# Add the Chi2-matcher to path
chi2Matcher = register_module("Chi2MCTrackMatcher")

# Define input parameters

# Chi2 Cut Off values.
# These are the reasonable default Cut Off values:
CutOffs = [128024, 95, 173, 424, 90, 424]
chi2Matcher.param("CutOffs", CutOffs)
# Package used for inversion of the covariance matrix:
# ROOT is default since in general it is faster and scales better
# False: ROOT [default]; True: eigen
chi2Matcher.param("linalg", False)

# Shows some additional Debug messages
chi2Matcher.logging.log_level = LogLevel.DEBUG
chi2Matcher.logging.debug_level = 30

# Add the Chi2-matcher module to the execution path
path.add_module(chi2Matcher)
# process the path
process(path)
# show some module statistics
print(statistics)
