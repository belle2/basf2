# Loading some common libraries
from ROOT import std
from ROOT import Belle2  # make Belle2 namespace available
import basf2
from ROOT import gSystem
gSystem.Load('libcdc')
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')
gSystem.Load('libgenfit2')
