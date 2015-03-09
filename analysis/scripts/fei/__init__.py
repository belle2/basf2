"""
Full Event Interpretation framework for Belle II
Thomas Keck, Christian Pulvermacher 2013-2015

Detailed usage examples can be found in analysis/examples/FEI/
"""

# @cond internal
__all__ = ['Particle', 'FeiState', 'fullEventInterpretation', 'get_default_channnels']  # only public interfaces, to make package doc nicer. Also the only things imported by 'from fei import *'
# @endcond

from fei import Particle
from fei import FeiState
from fei import fullEventInterpretation
from default_channels import get_default_channnels
