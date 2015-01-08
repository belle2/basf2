"""
Full Event Interpretation framework for Belle II
Thomas Keck, Christian Pulvermacher 2013-2014

Detailed usage examples can be found in analysis/examples/FEI/
"""

__all__ = ['Particle', 'fullEventInterpretation']  # only public interfaces, to make package doc nicer. Also the only things imported by 'from FEI import *'

from FEI import Particle
from FEI import fullEventInterpretation
