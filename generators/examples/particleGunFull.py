#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# This steering file which shows all usage options for the
# particle gun moduile in the generators package.
# The generated particles from the particle gun are the.
# fed through a full Geant4 simulation and the output
# is stored in a root file.
#
# The different options for the particle gun are explained
# below.
# Uncomment/comment different lines to get the wanted
# settings
#
# Example steering file - 2011 Belle II Collaboration
########################################################

from basf2 import *

# suppress messages and warnings during processing:
set_log_level(3)

# to run the framework the used modules need to be registered
pguninput = register_module('PGunInput')

# setting the options for the particle gun:

# setting the random seed for particle generation:
# this number can be any int, preferably large
# default is 3452346
pguninput.param('Rseed', 1028307)

# setting the number of tracks to be generated per event:
# this number can be any int>0
# default is 1
pguninput.param('ntracks', 10)

# setting the list of particle codes (PDG codes) for the generated particles
# the codes are given in an array,
# if only one code is used, the brackets should be kept:
# pguninput.param('PIDcodes',[11])
# there is no limit on how many codes can be given
# the particle gun will select randomly amongst the PDGcodes
# using a uniform distribuion
# default is [11,-11]
pguninput.param('PIDcodes', [11, -11])

# setting the parameters for the random generation
# of particles momenta:
# Three different distributions can be used:
# uniform momentum distribution (this is set by default)
pguninput.param('momentumGeneration', 0)
# in this case the two parameters for the momentum are
# used as lower (p_par1) and upper (p_par2) limits
pguninput.param('p_par1', 0.05)
pguninput.param('p_par2', 3)
# defaults are p_par1 = 0.2 and p_par2 = 1 GeV

# gaussion momentum distribution
# pguninput.param('momentumGeneration',1)
# in this case the two parameters for the momentum are
# used as mean (p_par1) and sigma (p_par2) of the gauss
# pguninput.param('p_par1',0.2)
# pguninput.param('p_par2',0.02)

# fixed momentum distribution
# pguninput.param('momentumGeneration',2)
# in this case the first parameter for the momentum is
# used to fix the momentum (p_par1). p_par2 is not used
# pguninput.param('p_par1',1)
# pguninput.param('p_par2',1010101)

# setting the parameters for the random generation
# of the particle direction:
# The two angles theta and phi are generated according
# to a common choice of distributions
# One of three random distribution can be used:
# uniform distribution of angles
pguninput.param('angleGeneration', 0)  # (this is default)
# in this case the two parameters for each angle are
# used as lower (ph_par1,th_par1) and upper (ph_par2,th_par2) limits
pguninput.param('th_par1', 37)
pguninput.param('th_par2', 120)
pguninput.param('ph_par1', 90)
pguninput.param('ph_par2', 95)
# default values are:
# ph_par1 = 0   ph_par2 = 360
# th_par1 = 17  th_par2 = 150

# gaussian angle distribution
# pguninput.param('angleGeneration',1)
# in this case the two parameters for each angle are
# used as mean (ph_par1,th_par1) and sigma (ph_par2,th_par2).
pguninput.param('th_par1', 39)
pguninput.param('th_par2', 2)
pguninput.param('ph_par1', 331)
pguninput.param('ph_par2', 10)
# default values are:
# ph_par1 = 0   ph_par2 = 360
# th_par1 = 17  th_par2 = 150

# fixed angles
# pguninput.param('angleGeneration',2)
# in this case only the first parameter for each angle is
# used (ph_par1,th_par1) and the second is ignored.
pguninput.param('th_par1', 35)
pguninput.param('ph_par1', 263)
# default values are:
# ph_par1 = 0
# th_par1 = 17

# setting the parameters for the random generation
# of event vertex:
# The vertices (starting point of all tracks) are generated
# in three coordinates. One of three random distribution
# can be used:
# uniform position distribution
pguninput.param('vertexGeneration', 0)
# in this case the two parameters for each coordinate are
# used as lower (x_par1) and upper (x_par2) limits
pguninput.param('x_par1', -0.5)
pguninput.param('x_par2', 0.5)
pguninput.param('y_par1', -0.5)
pguninput.param('y_par2', 0.5)
pguninput.param('z_par1', -2.5)
pguninput.param('z_par2', 2.5)
# default values are:
# x_par1 = y_par1 = z_par1= 0.0
# x_par2 = y_par2 = 0.7
# z_par2 = 1

# gaussian position distribution
# pguninput.param('vertexGeneration',1)
# in this case the two parameters for each coordinate are
# used as mean (x_par1) and sigma (x_par2) of the gaussian
# pguninput.param('x_par1',0.0)
# pguninput.param('x_par2',0.2)
# pguninput.param('y_par1',0.0)
# pguninput.param('y_par2',0.2)
# pguninput.param('z_par1',0)
# pguninput.param('z_par2',2)
# default values are:
# x_par1 = y_par1 = z_par1= 0.0
# x_par2 = y_par2 = 0.7
# z_par2 = 1

# fixed  position
# pguninput.param('vertexGeneration',2) (this is the default)
# in this case only the first parameter for each coordinate are
# used. The second parameters are ignored and don't need to be set.
# pguninput.param('x_par1',0.0)
# pguninput.param('y_par1',0.0)
# pguninput.param('z_par1',0)
# default values are:
# x_par1 = y_par1 = z_par1= 0.0

# for a simple simulation job with output to a root file
# these additional modules are needed
evtmetagen = register_module('EvtMetaGen')
paramloader = register_module('ParamLoaderXML')
geobuilder = register_module('GeoBuilder')
g4sim = register_module('FullSim')
simpleoutput = register_module('SimpleOutput')

# Setting the option for all non particle gun modules:
# want to process 100 MC events
evtmetagen.param('EvtNumList', [100])

paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

simpleoutput.param('outputFileName', 'ParticleGunOutput.root')

# creating the path for the processing

main = create_path()
main.add_module(evtmetagen)

# Add Particle Gun module to path:
main.add_module(pguninput)
# and print parameters for particle gun
# on startup of process
print_params(pguninput)

# Add all other modules for simple processing to path
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(g4sim)
main.add_module(simpleoutput)

# Process events
process(main)

