# Loading some common libraries
from ROOT import std  # noqa
from ROOT import Belle2  # make Belle2 namespace available # noqa
import basf2  # noqa
from ROOT import gSystem
gSystem.Load('libcdc')
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')
gSystem.Load('libgenfit2')
