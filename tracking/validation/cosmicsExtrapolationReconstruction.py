#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>CosmicsSimNoBkg.root</input>
  <output>CosmicsExtrapolation.root</output>
  <description>Validation of cosmic track extrapolation.</description>
</header>
"""

import basf2
from reconstruction import add_cosmics_reconstruction

ACTIVE = True


def run():
    """
    Run the cosmics reconstruction and extrapolation.
    """
    basf2.set_random_seed(12345)

    main = basf2.create_path()

    # Input.
    main.add_module('RootInput', inputFileName='../CosmicsSimNoBkg.root')

    # Extrapolation. Tracks are not merged (otherwise, many backward-extrapolated
    # tracks are removed).
    main.add_module('Gearbox')
    main.add_module('Geometry')
    add_cosmics_reconstruction(main, merge_tracks=False)

    # Output.
    output = basf2.register_module('RootOutput')
    output.param('outputFileName', '../CosmicsExtrapolation.root')
    output.param('branchNames', ['ExtHits', 'KLMHit2ds',
                                 'MCParticles', 'Tracks', 'TrackFitResults'])
    main.add_module(output)

    main.add_module('Progress')

    # Process the path.
    basf2.process(main, max_event=1000)

    print(basf2.statistics)


if __name__ == '__main__':
    if ACTIVE:
        run()
    else:
        print("This validation deactivated and thus basf2 is not executed.\n"
              "If you want to run this validation, please set the 'ACTIVE' flag above to 'True'.\n"
              "Exiting.")
