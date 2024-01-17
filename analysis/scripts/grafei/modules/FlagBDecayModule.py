import basf2 as b2
from ROOT import Belle2


def get_object_list(pointerVec):
    """
    Workaround to avoid memory problems in basf2.

    Args:
        pointerVec (list or Belle2.ParticleList): Input particle list.

    Returns:
        list: Output python list.
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
    Adds to particles in a given particle list the array index of the generated B meson ancestor, if a MC matching is found.
    If the particle list contains non-FSPs, the FSPs are retrieved.

    .. note::
        Assumes MC matching has been run on the particle list.
        When processing MC, this module must be put necessarily before `GraFEISaverModule` in order to save MC-truth information.

    Args:
        particle_list (str): Name of source particle list.
        b_parent_var (str): Name of the extraInfo to save.
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
        """"""
        p_list = get_object_list(Belle2.PyStoreObj(self.particle_list).obj())

        particles = []
        for p in p_list:
            particles.extend(get_object_list(p.getFinalStateDaughters()))

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
