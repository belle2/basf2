
from basf2 import *

path = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [5, 1])
eventinfosetter.param('runList', [0, 1])
eventinfosetter.param('expList', [0, 1])
path.add_module(eventinfosetter)

pgun = register_module('ParticleGun')
pgun.param('nTracks', 3)
path.add_module(pgun)

from ROOT import Belle2


class TestModule(Module):
    """print some debug info"""

    def event(self):
        """reimplementation of Module::event()."""

        part = Belle2.PyStoreObj('MCParticle')
        B2INFO("MCPart: " + str(part.obj().getIndex()))

    def beginRun(self):
        """reimplementation of Module::beginRun()."""
        B2INFO("TestModule: beginRun()")


subeventpath = create_path()
subeventpath.add_module(register_module('EventInfoPrinter'))
subeventpath.add_module(TestModule())
#read: for each  $objName   in $arrayName   run over $path
path.for_each('MCParticle', 'MCParticles', subeventpath)

path.add_module(register_module('PrintCollections'))

print path
process(path)
#
#print "event"
#print statistics
#print "init"
#print statistics(statistics.INIT)
#print "beginRun"
#print statistics(statistics.BEGIN_RUN)
#print "endRun"
#print statistics(statistics.END_RUN)
#print "terminate"
#print statistics(statistics.TERM)
