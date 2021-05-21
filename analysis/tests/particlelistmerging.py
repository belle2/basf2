import basf2
import math
import random
import modularAnalysis as ma
from ROOT import Belle2

# disable doxygen check for this test
# @cond


class Generator(basf2.Module):
    """Generate list of 10 electrons which have random momenta and one
    electron where all momentum components are nan"""

    def initialize(self):
        """We need to register the mc particles"""
        #: MCParticle array
        self.mcp = Belle2.PyStoreArray("MCParticles")
        self.mcp.registerInDataStore()

    def event(self):
        """And then we generate particles"""
        for i in range(10):
            p = self.mcp.appendNew()
            p.setPDG(11)
            p.setMassFromPDG()
            p.setMomentum(random.randrange(0, 5), random.randrange(0, 5), random.randrange(0, 5))

        p = self.mcp.appendNew()
        p.setPDG(11)
        p.setMassFromPDG()
        p.setMomentum(math.nan, math.nan, math.nan)


class MergingChecker(basf2.Module):
    """Check if merging works correctly"""

    def initialize(self):
        """Create particle list objects"""
        #: ParticleList objects
        self.all = Belle2.PyStoreObj('e-:allMC')
        self.below2 = Belle2.PyStoreObj('e-:below2')
        self.above3 = Belle2.PyStoreObj('e-:above3')
        self.onlyall = Belle2.PyStoreObj('e-:onlyall')
        self.goodmerge = Belle2.PyStoreObj('e-:goodmerge')
        self.best = Belle2.PyStoreObj('e-:best')

    def event(self):
        """Check if the particle lists have the expected size"""
        allsize = self.all.getListSize()
        below2size = self.below2.getListSize()
        above3size = self.above3.getListSize()
        onlyallsize = self.onlyall.getListSize()
        goodmergesize = self.goodmerge.getListSize()
        bestsize = self.best.getListSize()
        # The test fails if the number of particles in the merged lists differ from the sum of the desired input lists
        assert onlyallsize == allsize
        assert goodmergesize == below2size + above3size
        assert bestsize == below2size + above3size


# we create 10 events with 11 electrons each
path = basf2.Path()
path.add_module("EventInfoSetter", evtNumList=10)
path.add_module(Generator())
# load all electrons and add extraInfo so that they can be identified
ma.fillParticleListFromMC("e-:allMC", "", path=path)
ma.variablesToExtraInfo('e-:allMC', {'formula(0)': 'identifier', 'random': 'quality'}, path=path)
# load electrons with px below 2
ma.fillParticleListFromMC("e-:below2", "px < 2", path=path)
ma.variablesToExtraInfo('e-:below2', {'formula(2)': 'identifier', 'formula(1 + random)': 'quality'}, option=2, path=path)
# load electrons with px above 3
ma.fillParticleListFromMC("e-:above3", "px > 3", path=path)
ma.variablesToExtraInfo('e-:above3', {'formula(3)': 'identifier', 'formula(2 + random)': 'quality'}, option=2, path=path)

# In the momentum range 0 < px < 2 the electrons from the list e-:below2 have the highest quality value
# Electrons with a x-momentum between 2 and 3 are only part of the e-:all list.
# For electrons with x-momenta above 3 the candidates from the e-:above3 list have the highest quality value

# Merging the three lists with the e-:all list as first input list
ma.copyLists('e-:onlyall', ['e-:allMC', 'e-:below2', 'e-:above3'], path=path)
# Since e-:all is a superset of the other two lists, the merged list should contain only particles with identifier equals 0.
ma.printVariableValues('e-:onlyall', ['extraInfo(identifier)'], path=path)
# ma.applyCuts('e-:onlyall', 'extraInfo(identifier) == 0', path=path)

# Merging the three lists with the e-:all list at the end
ma.copyLists('e-:goodmerge', ['e-:below2', 'e-:above3', 'e-:allMC'], path=path)
# All electrons from the e-:below2 and e-:above3 lists should be present in
# the merged list, and those with 2 < px < 3 from the e-:all list.
# Thus, the merged list contains the ten best (according to quality)
# particles. This desired outcome was achieved accidentally.
ma.printVariableValues('e-:goodmerge', ['extraInfo(identifier)'], path=path)
ma.applyCuts('e-:goodmerge', 'extraInfo(identifier) > 0', path=path)

# Merging the three lists with the e-:all list as first input list but providing the quality extra info as sorting variable
path.add_module('ParticleListManipulator', outputListName='e-:best', inputListNames=['e-:allMC', 'e-:below2', 'e-:above3'],
                variable='extraInfo(quality)', preferLowest=False)
# The merged list should again contain the ten best particles. This time, independent of the order in the input list.
ma.printVariableValues('e-:best', ['extraInfo(identifier)'], path=path)
ma.applyCuts('e-:best', 'extraInfo(identifier) > 0', path=path)

path.add_module(MergingChecker())

basf2.process(path)

# @endcond
