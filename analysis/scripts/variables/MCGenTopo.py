#!/usr/bin/env python3
from variables.utils import create_aliases
from variables import variables


def mc_gen_topo(n=200):
    """
    Gets the list of variables containing the raw topology information of MC generated events.
    To be specific, the list including the following variables:

    * ``nMCGen``: number of MC generated particles in a given event,
    * ``MCGenPDG_i`` (i=0, 1, ... n-2, n-1):
      PDG code of the :math:`{\\rm i}^{\\rm th}` MC generated particle in a given event,
    * ``MCGenMothIndex_i`` (i=0, 1, ... n-2, n-1):
      mother index of the :math:`{\\rm i}^{\\rm th}` MC generated particle in a given event.

    .. tip::

       * Internally, ``nMCGen``, ``MCGenPDG_i`` and ``MCGenMothIndex_i`` are just aliases of
         ``nMCParticles``, ``genParticle(i, varForMCGen(PDG))``
         and ``genParticle(i, varForMCGen(mcMother(mdstIndex)))``, respectively.
       * For more details on the variables, please refer to the documentations of
         :b2:var:`nMCParticles`, :b2:var:`genParticle`, :b2:var:`varForMCGen`,
         :b2:var:`PDG`, :b2:var:`mcMother`, and :b2:var:`mdstIndex`.

    Parameters:
        n (int): number of ``MCGenPDG_i``/``MCGenMothIndex_i`` variables. Its default value is 200.

    .. note::

       * To completely examine the topology information of the events in an MC sample,
         the parameter ``n`` should be greater than or equal to the maximum of ``nMCGen`` in the sample.
       * Normally, the maximum of ``nMCGen`` in the MC samples at Belle II is less than 200.
         Hence, if you have no idea about the maximum of ``nMCGen`` in your own MC sample,
         it is usually a safe choice to use the default parameter value 200.
       * However, an overlarge parameter value leads to unncessary waste of disk space and
         redundant variables with inelegant ``nan`` values.
         Hence, if you know the maximum of ``nMCGen`` in your own MC sample,
         it is a better choice to assign the parameter a proper value.
    """
    list_of_indexes = [str(i) for i in range(n)]
    wrapper = 'genParticle({variable}, varForMCGen(PDG))'
    prefix = 'MCGenPDG'
    MCGenPDG = create_aliases(list_of_indexes, wrapper, prefix)
    wrapper = 'genParticle({variable}, varForMCGen(mcMother(mdstIndex)))'
    prefix = 'MCGenMothIndex'
    MCGenMothIndex = create_aliases(list_of_indexes, wrapper, prefix)
    variables.addAlias('nMCGen', 'nMCParticles')
    list_of_variables = ['nMCGen']
    for i in range(n):
        list_of_variables.append(MCGenPDG[i])
        list_of_variables.append(MCGenMothIndex[i])
    return list_of_variables
