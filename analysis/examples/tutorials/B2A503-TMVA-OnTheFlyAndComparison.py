#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to perform a
# TMVA training on electron candidates using the TMVAOnTheFlyTeacher.
# This module combines the TMVATeacher and the externTeacher tool.
#
# It's also possible to train multiple methods at once with this module,
# and compare the different methods using the showTMVAResults tool.
#
# Note: This example uses the generic MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
#
# Contributors: T. Keck (June 2015)
#
######################################################

from basf2 import *
from modularAnalysis import *

filelist = [
    '/local/ssd-scratch/tkeck/mc/mc35_charged_BGx0_s00/charged_e0001r000*.root',
    # '/hsm/belle2/bdata/MC/generic/mixed/mcprod1405/BGx1/mc35_mixed_BGx1_s00/mixed_e0001r0010_s00_BGx1.mdst.root',
    # '/hsm/belle2/bdata/MC/generic/charged/mcprod1405/BGx1/mc35_charged_BGx1_s00/charged_e0001r0010_s00_BGx1.mdst.root',
    # '/hsm/belle2/bdata/MC/generic/ccbar/mcprod1405/BGx1/mc35_ccbar_BGx1_s00/ccbar_e0001r0010_s00_BGx1.mdst.root',
    # '/hsm/belle2/bdata/MC/generic/ssbar/mcprod1405/BGx1/mc35_ssbar_BGx1_s00/ssbar_e0001r0010_s00_BGx1.mdst.root',
    # '/hsm/belle2/bdata/MC/generic/ddbar/mcprod1405/BGx1/mc35_ddbar_BGx1_s00/ddbar_e0001r0010_s00_BGx1.mdst.root',
    # '/hsm/belle2/bdata/MC/generic/uubar/mcprod1405/BGx1/mc35_uubar_BGx1_s00/uubar_e0001r0010_s00_BGx1.mdst.root'
]

main = create_path()
inputMdstList('MC5', filelist, path=main)

# Select all electron candidates
fillParticleList('e-', '', False, path=main)

# One can do the data taking and training at the same time using the TMVAOnTheFlyTeacher
# The TMVAOnTheFlyTeacher takes additional arguments like target, prepareOption, methods
teacher = register_module('TMVAOnTheFlyTeacher')
# The TMVAOntheFlyTeacher the same arguments as the TMVATeacher module
teacher.param('workingDirectory', 'TMVA')
teacher.param('prefix', 'OnTheFly')
teacher.param('variables', ['eid', 'useCMSFrame(p)', 'chiProb'])
teacher.param('listNames', 'e-')
# And in addition it takes nearly the same arguments as the externTeacher module
# methodName, methodType and methodConfig are given as a tuple. It's possible to define more than one method!
teacher.param('methods',
              [('TMVABDT', 'BDT', '!H:!V:CreateMVAPdfs:NbinsMVAPdf=100:NTrees=100:BoostType=Grad:'
                'Shrinkage=0.10:GradBaggingFraction=0.5:nCuts=256:MaxDepth=3'),
               ('FastBDT', 'Plugin', '!H:!V:CreateMVAPdfs:NbinsMVAPdf=100:NTrees=100:Shrinkage=0.10:'
                'RandRatio=0.5:NCutLevel=8:NTreeLayers=3')])
# The target is automatically added as a spectator, so no need to do it by hand as before
teacher.param('target', 'isSignal')
main.add_module(teacher)

process(main)
print(statistics)


# Now lets apply the methods as usual

main = create_path()
inputMdstList('MC5', filelist, path=main)
fillParticleList('e-', '', False, path=main)

# Apply the TMVA training using the TMVAExpert module
expert = register_module('TMVAExpert')
expert.param('workingDirectory', 'TMVA')
expert.param('prefix', 'OnTheFly')
expert.param('method', 'FastBDT')
expert.param('expertOutputName', 'FastBDTProbability')
expert.param('listNames', 'e-')
main.add_module(expert)

# There's also a convinience function which does the same,
# we use it to apply the second on-the-fly training.
applyTMVAMethod('e-', prefix='OnTheFly', method="TMVABDT", expertOutputName='TMVABDTProbability',
                workingDirectory="TMVA", path=main)

variablesToNTuple('e-', ['extraInfo(FastBDTProbability)', 'extraInfo(TMVABDTProbability)', 'isSignal'],
                  filename='TMVA/OnTheFlyOutput.root', path=main)

process(main)
print(statistics)
