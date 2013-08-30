from basf2 import *

import interactive

from ROOT import Belle2


class MinModule(Module):
    """
    Example module to drop into ipython and create some objects to look at.
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
        "\nTo continue non-interactively, press Ctrl+D.")
        B2INFO(80 * '=')
        interactive.embed()


main = create_path()

evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('evtNumList', [2])
main.add_module(evtmetagen)

pGun = register_module('ParticleGun')
main.add_module(pGun)

main.add_module(MinModule())

process(main)
