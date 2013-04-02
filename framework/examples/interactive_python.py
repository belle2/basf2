from basf2 import *

try:
    import IPython
    makeInteractive = IPython.embed
except:  # older ipython ver
    try:
        from IPython.Shell import IPShellEmbed
        makeInteractive = IPShellEmbed()
    except:  # no ipython at all
        B2FATAL("This example requires ipython."
        " For a portable, if limited, solution use:"
        "\nimport code"
        "\ncode.interact(local=locals())\n")


from ROOT import Belle2


class MinModule(Module):
    def __init__(self):
        super(MinModule, self).__init__()  # call constructor of base class

    def event(self):
        evtmetadata = Belle2.PyStoreObj("EventMetaData")
        particles = Belle2.PyStoreArray("MCParticles")
        B2INFO(80 * '=')
        B2INFO("Dropping into interactive python shell. Try:"
        "\n  print evtmetadata.obj().getEvent()"
        "\n  particles[0].Dump()"
        "\n  help(particles[0])"
        "\nTo continue non-interactively, press Ctrl+D.")
        B2INFO(80 * '=')
        makeInteractive()


main = create_path()

evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('EvtNumList', [2])
main.add_module(evtmetagen)

pGun = register_module('ParticleGun')
main.add_module(pGun)

main.add_module(MinModule())

process(main)
