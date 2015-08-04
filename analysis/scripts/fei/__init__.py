"""
Full Event Interpretation framework for Belle II
Thomas Keck, Christian Pulvermacher 2013-2015

Detailed usage examples can be found in analysis/examples/FEI/
"""

# @cond internal
# only public interfaces, to make package doc nicer. Also the only things imported by 'from fei import *'
__all__ = ['Particle', 'FeiState', 'fullEventInterpretation', 'get_default_channnels', 'MVAConfiguration',
           'PreCutConfiguration', 'PostCutConfiguration', 'DecayChannel', 'UserCutConfiguration', 'PlotConfiguration']
# @endcond

from fei.steering import Particle, MVAConfiguration, PreCutConfiguration, PostCutConfiguration, UserCutConfiguration
from fei.steering import DecayChannel, PlotConfiguration
from fei.steering import FeiState
from fei.steering import fullEventInterpretation
from fei.default_channels import get_default_channnels
