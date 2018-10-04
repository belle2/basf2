#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to use the Event Shape framework
#
# Contributors: Umberto Tamponi (October 2018)
# tamponi@to.infn.it
#
####################################################################


#
# Import and mdst loading
#

from basf2 import *
from modularAnalysis import *

# load input ROOT file
filelistSIG = '/group/belle2/tutorial/release_01-00-00/mdst-Y4setaY.root'

inputMdst('default', filelistSIG)

# Creates a list of good pions and good gammas with very minimal cuts
fillParticleList('pi+:all', 'pt> 0.1')
fillParticleList('gamma:all', 'E > 0.1')

# Builds the evt shape enabling ALL the variables
buildEventShape(inputListNames=['pi+:all', 'gamma:all'], allMoments=True)


variables = ['foxWolframR1',
             'foxWolframR2',
             'foxWolframR3',
             'foxWolframR4',
             'cleoCone(0,thrust)',
             'cleoCone(1,thrust)',
             'cleoCone(2,thrust)',
             'cleoCone(3,thrust)',
             'cleoCone(4,thrust)',
             'cleoCone(5,thrust)',
             'cleoCone(6,thrust)',
             'cleoCone(7,thrust)',
             'cleoCone(8,thrust)',
             'multipoleMoment(0,thrust)',
             'multipoleMoment(1,thrust)',
             'multipoleMoment(2,thrust)',
             'multipoleMoment(3,thrust)',
             'multipoleMoment(4,thrust)',
             'multipoleMoment(5,thrust)',
             'multipoleMoment(6,thrust)',
             'multipoleMoment(7,thrust)',
             'multipoleMoment(8,thrust)',
             'multipoleMoment(0,collision)',
             'multipoleMoment(1,collision)',
             'multipoleMoment(2,collision)',
             'multipoleMoment(3,collision)',
             'multipoleMoment(4,collision)',
             'multipoleMoment(5,collision)',
             'multipoleMoment(6,collision)',
             'multipoleMoment(7,collision)',
             'multipoleMoment(8,collision)',
             'foxWolframR(0)',
             'foxWolframR(1)',
             'foxWolframR(2)',
             'foxWolframR(3)',
             'foxWolframR(4)',
             'foxWolframR(5)',
             'foxWolframR(6)',
             'foxWolframR(7)',
             'foxWolframR(8)',
             'foxWolframH(0)',
             'foxWolframH(1)',
             'foxWolframH(2)',
             'foxWolframH(3)',
             'foxWolframH(4)',
             'foxWolframH(5)',
             'foxWolframH(6)',
             'foxWolframH(7)',
             'foxWolframH(8)',
             'backwardEmisphereMass',
             'forwardEmisphereMass',
             'sphericity',
             'aplanarity',
             'thrust',
             'thrustAxisCosTheta',
             'R2EventLevel']

variablesToNtuple('', variables, filename='B2A704-EventShape.root')


# Process the events
process(analysis_main)
# print out the summary
print(statistics)
