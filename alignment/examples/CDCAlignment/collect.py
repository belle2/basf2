#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *
from ROOT import Belle2
"""
Belle2.GlobalLabel.disableComponent(Belle2.VXDAlignment.getGlobalUniqueID())
Belle2.GlobalLabel.disableComponent(Belle2.BeamParameters.getGlobalUniqueID())
Belle2.GlobalLabel.disableComponent(Belle2.CDCTimeZeros.getGlobalUniqueID())
Belle2.GlobalLabel.disableComponent(Belle2.CDCTimeWalks.getGlobalUniqueID())
Belle2.GlobalLabel.disableComponent(Belle2.BKLMAlignment.getGlobalUniqueID())
Belle2.GlobalLabel.disableComponent(Belle2.EKLMAlignment.getGlobalUniqueID())
"""
main = create_path()

main.add_module('RootInput')
main.add_module('HistoManager', histoFileName='CollectorOutput.root')
main.add_module('Gearbox')
main.add_module('Geometry')
main.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)
main.add_module('DAFRecoFitter')
main.add_module('MillepedeCollector', minPValue=0, components=['CDCAlignment', 'CDCLayerAlignment'])
main.add_module('Progress')
process(main)

print(statistics)
