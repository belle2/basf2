"""
Full Event Interpretation framework for Belle II
Thomas Keck, Christian Pulvermacher 2013-2014

Detailed usage examples can be found in analysis/examples/FEI/
"""

# @cond internal
__all__ = ['Particle', 'fullEventInterpretation', 'get_default_channnels']  # only public interfaces, to make package doc nicer. Also the only things imported by 'from FEI import *'
# @endcond

from FEI import Particle
from FEI import fullEventInterpretation
from default_channels import get_default_channnels
