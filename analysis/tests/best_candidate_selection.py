##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2
import math
import random
from collections import defaultdict
import modularAnalysis as ma
from ROOT import Belle2


class Generator(basf2.Module):
    """Generate a list of 10 electrons which have stupid momenta just to sort
    them later. And then add one electron where all momentum components are
    nan"""

    def initialize(self):
        """We need to register the mc particles"""
        #: MCParticle array
        self.mcp = Belle2.PyStoreArray("MCParticles")
        self.mcp.registerInDataStore()

    def event(self):
        """And then we generate particles, ensuring some overlap in the momenta."""
        print("New event:")
        for _ in range(5):
            px, py, pz = random.randrange(1, 5), random.randrange(1, 5), random.randrange(1, 5)
            for _ in range(2):
                p = self.mcp.appendNew()
                p.setPDG(11)
                p.setMassFromPDG()
                p.setMomentum(px, py, pz)

        p = self.mcp.appendNew()
        p.setPDG(11)
        p.setMassFromPDG()
        p.setMomentum(math.nan, math.nan, math.nan)


class RankChecker(basf2.Module):
    """Check if the ranks are actually what we want"""

    def initialize(self):
        """Create particle list object"""
        #: particle list object
        self.plist = Belle2.PyStoreObj("e-")

    def event(self):
        """And check all the ranks"""
        # make a list of all the values and a dict of all the extra infos
        px = []
        py = []
        einfo = defaultdict(list)
        for particle in self.plist:
            px.append(particle.getPx())
            py.append(particle.getPy())
            # get all names of existing extra infos but convert to a native list of python strings to avoid
            # possible crashes if the std::vector returned by the particle goes out of scope
            names = [str(n) for n in particle.getExtraInfoNames()]
            for n in names:
                einfo[n].append(particle.getExtraInfo(n))

        # check the default name is set correctly if we don't specify an output variable
        print(list(einfo.keys()))
        assert 'M_rank' in einfo.keys(), "Default name is not as expected"

        # Now determine the correct ranks if multiple values are allowed:
        # create a dictionary which will be value -> rank for all unique values
        # in theory we just need loop over the sorted(set(values)) but we have
        # special treatment for nans which should go always to the end of the
        # list so sort with a special key that replaces nan by inf or -inf
        # depending on sort order
        px_value_ranks = {v: i for i, v in enumerate(sorted(set(px), reverse=True,
                                                            key=lambda v: -math.inf if math.isnan(v) else v), 1)}
        py_value_ranks = {v: i for i, v in enumerate(sorted(set(py),
                                                            key=lambda v: math.inf if math.isnan(v) else v), 1)}

        # Ok, test if the rank from extra info actually corresponds to what we
        # want
        for v, r in zip(px, einfo["px_high_multi"]):
            print(f"Value: {v}, rank: {r}, should be: {px_value_ranks[v]}")
            assert r == px_value_ranks[v], "Rank is not correct"

        for v, r in zip(py, einfo["py_low_multi"]):
            print(f"Value: {v}, rank: {r}, should be: {py_value_ranks[v]}")
            assert r == py_value_ranks[v], "Rank is not correct"

        # so we checked multiRank=True. But for multiRank=False this is more
        # complicated because ranking a second time will destroy the order
        # of the previous sorts. But we can at least check if all the ranks
        # form a range from 1..n if we sort them
        simple_range = list(range(len(px)))
        px_single_ranks = list(sorted(int(r) - 1 for r in einfo["px_high_single"]))
        assert simple_range == px_single_ranks, "sorted ranks don't form a range from 1..n"
        # but the second two rankings are on the same variable in the same
        # order so they need to keep the order stable. so for py_low_single the
        # ranks need to be the range without sorting
        py_single_ranks = list(int(r) - 1 for r in einfo["py_low_single"])
        assert simple_range == py_single_ranks, "ranks don't form a range from 1..n"


class NumBestChecker(basf2.Module):
    """Check if 'numBest' works correctly"""

    def __init__(self):
        """Initializing the parameters."""
        super().__init__()
        #: Number of candidates to keep (must be given as parameter, otherwise assert will fail).
        self.num_best = None
        #: MultiRank option switch
        self.allow_multirank = False

    def param(self, kwargs):
        """Checking for module parameters to distinguish between the different test cases."""
        self.num_best = kwargs.pop('numBest')
        self.allow_multirank = kwargs.pop('allowMultiRank', False)
        super().param(kwargs)

    def initialize(self):
        """Create particle list 'e-:numBest(MultiRank)' object, depending on parameter choice."""
        if self.allow_multirank:
            #: particle list object
            self.plist = Belle2.PyStoreObj('e-:numBestMultiRank')
        else:
            #: particle list object
            self.plist = Belle2.PyStoreObj('e-:numBest')

    def event(self):
        """Check if 'e-:numBest' and 'e-:numBestMultiRank' have the expected size"""

        size = self.plist.getListSize()
        if self.allow_multirank:
            px = [particle.getPx() for particle in self.plist]
            px_value_ranks = {v: i for i, v in enumerate(sorted(set(px), reverse=True,
                                                                key=lambda v: -math.inf if math.isnan(v) else v),
                                                         1)}
            remaining_particles = [v for v in px if px_value_ranks[v] <= self.num_best]
            assert size <= len(remaining_particles), "numBest test with multirank failed: " \
                f"there should be {len(remaining_particles)} Particles in the list " \
                f"instead of {size}!"

        else:
            # The test fails if size > numBest_value as this is passed as a parameter into the module
            assert size <= self.num_best, f"numBest test failed: there are too many Particles ({size}) in the list!"


# fixed random numbers
random.seed(5)
# so lets create 10 events
path = basf2.Path()
path.add_module("EventInfoSetter", evtNumList=10)
# and put some electrons in there
path.add_module(Generator())
# load these electrons
ma.fillParticleListFromMC("e-", "", path=path)
# and sort them ...
ma.rankByHighest("e-", "M", path=path)
ma.rankByHighest("e-", "px", allowMultiRank=False, outputVariable="px_high_single", path=path)
ma.rankByHighest("e-", "px", allowMultiRank=True, outputVariable="px_high_multi", path=path)
ma.rankByLowest("e-", "py", allowMultiRank=False, outputVariable="py_low_single", path=path)
ma.rankByLowest("e-", "py", allowMultiRank=True, outputVariable="py_low_multi", path=path)
# and also check sorting
path.add_module(RankChecker())

# we set numBest = 2: this is used also for the assert
numBest_value = 2

# create a new list
ma.fillParticleListFromMC("e-:numBest", "", path=path)
# sort the list, using numBest
ma.rankByHighest("e-:numBest", "p", numBest=numBest_value, path=path)
# and check that numBest worked as expected
path.add_module(NumBestChecker(), numBest=numBest_value)

# create another new list, this time for multi rank test
ma.fillParticleListFromMC("e-:numBestMultiRank", "", path=path)
# sort the list, using numBest and allowMultiRank
ma.rankByHighest("e-:numBestMultiRank", "px", numBest=numBest_value, allowMultiRank=True, path=path)
# and check that numBest worked as expected
path.add_module(NumBestChecker(), numBest=numBest_value, allowMultiRank=True)

basf2.process(path)
