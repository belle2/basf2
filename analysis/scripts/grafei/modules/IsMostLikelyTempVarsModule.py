import basf2 as b2
from variables import variables as vm
from ROOT import Belle2
import numpy as np
from grafei.modules.FlagBDecayModule import get_object_list


class IsMostLikelyTempVars(b2.Module):
    """
    Module to compute isMostLikely with temporary _noSVD and _noTOP likelihoods.

    .. note::
        This module is temporary, and will be removed when MC16 is available.

    Args:
        particle_lists (list): List of particle list names.
        priors (list): List of float priors.
    """

    def __init__(self, particle_lists, priors):
        super().__init__()
        self.particle_lists = particle_lists
        self.priors = priors

    def event(self):
        """"""
        # Loop over the particle lists
        for particle_list in self.particle_lists:
            p_name = particle_list.split(":")[0][:-1]
            particles = get_object_list(Belle2.PyStoreObj(particle_list).obj())

            # Loop over all the particles in the list
            for particle in particles:
                # List of variables needed during the process
                logl = []  # Log likelihoods list

                # Compute all the log Likelihoods
                # Electron_noSVD_noTOP
                logl.append(
                    vm.evaluate(
                        "pidLogLikelihoodValueExpert(11, CDC, ARICH, ECL, KLM)",
                        particle,
                    )
                )
                # Muon_noSVD
                logl.append(
                    vm.evaluate(
                        "pidLogLikelihoodValueExpert(13, CDC, TOP, ARICH, ECL, KLM)",
                        particle,
                    )
                )
                # Pion_noSVD
                logl.append(
                    vm.evaluate(
                        "pidLogLikelihoodValueExpert(211, CDC, TOP, ARICH, ECL, KLM)",
                        particle,
                    )
                )
                # Kaon_noSVD
                logl.append(
                    vm.evaluate(
                        "pidLogLikelihoodValueExpert(321, CDC, TOP, ARICH, ECL, KLM)",
                        particle,
                    )
                )
                # Proton_noSVD
                logl.append(
                    vm.evaluate(
                        "pidLogLikelihoodValueExpert(2212, CDC, TOP, ARICH, ECL, KLM)",
                        particle,
                    )
                )
                # Deuteron_noSVD
                logl.append(
                    vm.evaluate(
                        "pidLogLikelihoodValueExpert(1000010020, CDC, TOP, ARICH, ECL, KLM)",
                        particle,
                    )
                )

                log_likelihoods = np.array(logl)

                ll_max = np.max(log_likelihoods)  # Get the max
                norm = np.sum(np.exp(log_likelihoods - ll_max) * self.priors)  # Compute the norm
                probs = (np.exp(log_likelihoods - ll_max) * self.priors) / norm  # Compute the proba

                # Look at if the the index corresponding to the max proba is compatible with the list we are looking at
                ordered_names = [
                    "e",
                    "mu",
                    "pi",
                    "K",
                    "p",
                    "deuteron",
                ]
                particle.addExtraInfo(
                    "IsMostLikelyTempVars",
                    1 if ordered_names[np.argmax(probs)] == p_name else 0,
                )
