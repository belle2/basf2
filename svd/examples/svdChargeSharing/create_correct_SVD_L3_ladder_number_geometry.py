#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *

main = create_path()
main.add_module('EventInfoSetter')

main.add_module("Gearbox")
main.add_module('Geometry', components=['SVD'], createPayloads=True)

# NOTE: uncomment display to see what you get - but it does not work together with creating the payloads above
# main.add_module('Display', fullGeometry=True)

process(main)
print(statistics(statistics.BEGIN_RUN))
