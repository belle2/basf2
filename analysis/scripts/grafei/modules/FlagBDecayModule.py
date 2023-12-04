import basf2 as b2
from ROOT import Belle2


def getObjectList(pointerVec):
    """
    Workaround function to avoid memory problems in basf2.
    """
    objList = []
    size = (
        pointerVec.getListSize()
        if isinstance(pointerVec, Belle2.ParticleList)
        else len(pointerVec)
    )
    for i in range(size):
        objList.append(pointerVec[i])
    return objList


class FlagBDecayModule(b2.Module):
    """
    Saves variables indicating the decay mode the particles originated from.
    Flags indicated which B meson was the parent and whether the decay was semileptonic
    or hadronic.
    Assumes MC matching has already been run on the list.

    Args:
        particle_list (str): name of Belle II particle list (can be FSP or composite)
        b_parent_var (str): name of the extraInfo to save
    """

    def __init__(
        self,
        particle_list,
        b_parent_var='BParentGenID',
    ):
        super().__init__()
        self.particle_list = particle_list
        self.b_parent_var = b_parent_var

    def event(self):
        p_list = getObjectList(Belle2.PyStoreObj(self.particle_list).obj())

        particles = []
        for p in p_list:
            particles.extend(getObjectList(p.getFinalStateDaughters()))

        for particle in particles:
            # First get related MC particle
            # Need to add a condition here to flag non-matched particles
            b_ancestor = particle.getMCParticle()

            if not b_ancestor:
                # Set the extraInfo of the particle to reflect the B parent
                particle.addExtraInfo(self.b_parent_var, -1)
                continue

            # Now traverse upwards until we hit a B meson
            # Y4S=0, B1=1, B2=2, B1_daughter=3, etc.
            try:  # Sometimes b_ancestor does not have a mother when running on background MC
                while b_ancestor.getMother().getArrayIndex() > 0:
                    b_ancestor = b_ancestor.getMother()

                # Set the extraInfo of the particle to reflect the B parent
                if abs(b_ancestor.getPDG()) in [511, 521]:
                    particle.addExtraInfo(self.b_parent_var, b_ancestor.getArrayIndex())
                else:
                    particle.addExtraInfo(self.b_parent_var, -1)
            except BaseException:
                b2.B2DEBUG(
                    12,
                    f'MC particle (pdg code = {b_ancestor.getPDG()}) has no mother!')
                particle.addExtraInfo(self.b_parent_var, -1)

    def _is_semileptonic(self, mcp):
        """
        Recursive function to check if MCParticle is neutrino.

        If not then this calls itself on the daughters.
        Exits on first neutrino found.
        """
        for daughter in mcp.getDaughters():
            if self._is_semileptonic(daughter):
                return True

        if abs(mcp.getPDG()) in [12, 14, 16, 18]:
            return True

        return False
