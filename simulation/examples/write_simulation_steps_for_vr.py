##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
This example shows how to correctly write out the simulation steps into CSV files
for the Belle II Virtual Reality application.
'''

import basf2 as b2
import generators as ge
import simulation as si


# Write out the simulation steps into CSV files
b2.write_simulation_steps()


main = b2.Path()

main.add_module('EventInfoSetter')

# replace it with your favourite generator
ge.add_kkmc_generator(main, 'mu-mu+')

# the beam background files are "useless" for the virtual reality
# while it's fundamental to deactivate the T0 jitter simulation
si.add_simulation(main, simulateT0jitter=False)

main.add_module('Progress')

b2.process(main)
print(b2.statistics)
