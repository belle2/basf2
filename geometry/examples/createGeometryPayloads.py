#!/usr/bin/env python3

import basf2
main = basf2.create_path()
main.add_module("EventInfoSetter")
main.add_module("Gearbox")
main.add_module("Geometry", components=["MagneticField"], createPayloads=True)
basf2.process(main)
