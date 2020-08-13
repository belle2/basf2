#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../CharmlessHad2BodyCharged.dst.root</input>
    <output>../CharmlessHad2BodyCharged.udst.root</output>
    <contact>khsmith@student.unimelb.edu.au</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "K. Smith"

import basf2 as b2
import modularAnalysis as ma
from skim.btocharmless import CharmlessHad2Body

path = b2.Path()

fileList = ['../CharmlessHad2BodyCharged.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = CharmlessHad2Body(OutputFileName='../CharmlessHad2BodyCharged.udst.root')
skim(path)
b2.process(path)
