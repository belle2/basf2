# Path building
from basf2 import create_path, register_module, LogLevel, process, statistics

# analysis
from stdCharged import stdK, stdPi
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu


NtupleFilePath = "/nfs/dust/belle2/user/floschw/Data/example_Ntuple_1000.root"
ROOTFilePath = "/nfs/dust/belle2/user/floschw/Data/simulation/sim_1000.root"

path = create_path()
# Load a MDST Root file where relations from hit matching
# are excluded.
path.add_module("RootInput",
                inputFileName=ROOTFilePath,
                excludeBranchNames=["TracksToMCParticles"])

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

# Now you can do your reconstruction
# Here for example a reconstruction of a D*+ decay, for slow pion analysis

stdPi("all", path=path)
stdK("good", path=path)
decaystring = "anti-B0 -> [D*+ -> [[D0 -> K-:good pi+:good] pi+:good] e-:good anit-nu_e"
# reconstruct the D0. Introduced a mass cut to clean mass distribution.
ma.reconstructDecay("D0 -> K-:good pi+:all", "1.85466<M<1.875", path=path)
ma.matchMCTruth("D0", path=path)

# reconstruct the D*+
ma.reconstructDecay("D*+ -> D0 pi+:all", "", path=path)
ma.matchMCTruth("D*+", path=path)

# collect variables nessesary for slow pion analysis
listofvariables = [
    "E",
    "M",
    "PDG",
    "isSignalAcceptMissing",
    "d0",
    "d0Err",
    "omega",
    "omegaErr",
    "phi0",
    "phi0Err",
    "phi",
    "phiErr",
    "tanLambda",
    "tanLambdaErr",
    "z0",
    "z0Err",
    "theta",
    "thetaErr",
    "p",
    "pErr",
    "pt",
    "ptErr",
    "px",
    "pxErr",
    "py",
    "pyErr",
    "pz",
    "pzErr",
    "d0Pull",
    "omegaPull",
    "phi0Pull",
    "tanLambdaPull",
    "z0Pull",
    "mcP",
    "mcPT",
    "mcPX",
    "mcPY",
    "mcPZ",
    "mcDecayVertexX",
    "mcDecayVertexY",
    "mcDecayVertexZ",
    "isSignal",
    "mcErrors",
    "mcPDG",
    "charge"] + vc.mc_vertex

# assign the variables to dacay particles
variables = vu.create_aliases(listofvariables, "{variable}", "Dstar")
variables += vu.create_aliases(listofvariables, "daughter(0,{variable})", "D0")
variables += vu.create_aliases(listofvariables, "daughter(1,{variable})", "Dstarpi")

# save everything to an Ntuple
ma.variablesToNtuple("D*+",
                     variables=variables,
                     filename=NtupleFilePath,
                     treename="tree",
                     path=path)

# process the path
process(path)
# show some module statistics
print(statistics)
