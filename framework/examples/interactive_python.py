#!/usr/bin/env python3
# -*- coding: utf-8 -*-

############################################################
# Demonstrates using (I)python inside a basf2 session,
# creating a few example objects to look at.
# Prints further instructions when started.
############################################################

from basf2 import *
import interactive
from ROOT import Belle2


class MinModule(Module):
    """
    Example module to drop into ipython and create some objects to look at.
    If you just want to start IPython and create PyStoreArray etc.
    interactively in your own steering file, the 'Interactive' module
    might be of interest.
    """

    def event(self):
        """
        reimplement Module::event()
        """
        evtmetadata = Belle2.PyStoreObj("EventMetaData")
        particles = Belle2.PyStoreArray("MCParticles")
        B2INFO(80 * '=')
        B2INFO("Dropping into interactive python shell. Try:"
               "\n  print evtmetadata.obj().getEvent()"
               "\n  particles[0].Dump()"
               "\n  help(particles[0])"
               "\nTo continue non-interactively (until next event), press Ctrl+D."
               "\nPress Ctrl+C followed by Ctrl+D to exit basf2.")
        B2INFO(80 * '=')
        interactive.embed()


main = create_path()
main.add_module('EventInfoSetter', evtNumList=[2])
main.add_module('ParticleGun')
main.add_module(MinModule())
process(main)
