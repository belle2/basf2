#!/usr/bin/env python3
# -*- coding: utf-8 -*-

############################################################
# This steering file shows how to use overrides for xml files
#
# Example steering file - 2014 Belle II Collaboration
############################################################

from basf2 import *

main = create_path()
# EventInfoSetter - generate event meta data
main.add_module('EventInfoSetter', evtNumList=[1])

gearbox = main.add_module("Gearbox")
gearbox.param({
    # Set the length of the simulation volume to 9m and the width to 3m and the
    # Material to Vacuum instead of Air (no unit for the Material though)
    'override': [
        ("/Global/length", "9", "m"),
        ("/Global/width", "3", "m"),
        # We don't need a unit so we supply an empty one
        ("/Global/material", "Vacuum", ""),
        # Ok, so now we change the ActiveChips parameter of the PXD to true.
        # The // matches any number of elements in between, so the Material
        # could be somewhere deep in the xml but we do not need to care. //foo
        # matches /foo, /a/foo, /b/foo, /a/b/foo and so forth
        # foo[@name='bar'] matches all elements <foo> which have an attribute
        # name with the value bar (the @ is to specify attributes)
        # So, this line reads "override all ActiveChips elements somewhere
        # inside an DetectorComponent element which has the name attribute set
        # to PXD". If more than one element matches this expression an error is
        # produced to be on the safe side.
        ("/DetectorComponent[@name='PXD']//ActiveChips", "true", ""),
    ],
    # A normal override can affect only one parameter. If one wants to override
    # a number of parameters at once one has to use overrideMultiple to
    # indicate that this is really the intention. So let's change the element
    # fractions in Vacuum to all be equal to one
    # //@fraction selects the fraction attributes which are descendents of the
    # the Material element. Attributes cannot have a unit so we need to supply
    # an empty unit
    'overrideMultiple': [
        ("//Material[@name='Vacuum']//@fraction", "1.0", "")
    ],
})

# If we would want to modify many parameters of one subdetector we could set
# the override prefix to have more convinient access. Beware, the override
# prefix is the same for all overrides so this would interfere with the
# examples above

# gearbox.param({
#    "overridePrefix": "//DetectorComponent[@name='PXD']/Content/",
#    "override": [
#        ("ActiveChips", "true", ""),
#        ("SeeNeutrons", "true", ""),
#        ("SensitiveThreshold", "0", "eV"),
#    ],
# })

# Process all events
process(main)
print(statistics(statistics.BEGIN_RUN))
