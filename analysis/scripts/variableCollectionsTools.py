#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from functools import reduce
import collections as co


def create_aliases(list_of_variables, wrapper, prefix):
    """
    The function creates aliases for variables from the variables list with given wrapper
    and returns list of the aliases.

    >>> list_of_variables = ['M','p']
    >>> wrapper = 'daughter(1,{variable})'
    >>> prefix = 'pref'
    >>> print(create_aliases(list_of_variables, wrapper, prefix))
    ['pref_M', 'pref_p']
    >>> from variables import variables
    >>> variables.printAliases()
    [INFO] =========================
    [INFO] Following aliases exists:
    [INFO] 'pref_M' --> 'daughter(1,M)'
    [INFO] 'pref_p' --> 'daughter(1,p)'
    [INFO] =========================

    Parameters:
        list_of_variables (list(str)): list of variable names
        wrapper (str): metafunction taking variables from list_of_variables as a parameter \
        (``<metafunction>(<some configs>, {variable} ,<some otehr configs>)``
        prefix (str): alias prefix used for wrapped variables.

    Returns:
        list(str): new variables list
    """
    from variables import variables
    aliases = [f"{prefix}_{e}" for e in list_of_variables]
    for var, alias in zip(list_of_variables, aliases):
        variables.addAlias(alias, wrapper.format(variable=var))

    return aliases


def get_hierarchy_of_decay(decay_string):
    """
    This function returns hierarchy pathes of the particles selected in decay string.
    In C++ part of the basf2, hierarchy path is vector of pairs of relative daughter numbers and particle names.
    For instance, in decay
    ``B+ -> [ D+ -> ^K+ pi0 ] pi0``
    decay path of K+ is
    ``[(0, D), (0 K)]``:

    >>> print(get_hierarchy_of_decay('B+ -> [ D+ -> ^K+ pi0 ] pi0'))
    [[(0, 'D'), (0, 'K')]]

    Every selected partcile has its own hierarchy path and
    they are stored as a vector in this variable:
    For the decayString
    ``B+ -> [ D+ -> ^K+ pi0 ] ^pi0``
    m_hierarchy, once filled, is
    ``[[(0, D), (0, K)],
    [(1, pi0)]]``:

    >>> print(get_hierarchy_of_decay('B+ -> [ D+ -> ^K+ pi0 ] ^pi0'))
    [[(0, 'D'), (0, 'K')], [(1, 'pi0')]]

    Note: here C++ object are converted to python ones (vector of pairs becomes list of tuples)

    Note: hierarchy path here is slightly different from one in DecayDescriptor class.
    The difference is that the first element in all pathes (common mother particle) is ommited

    Parameters:
        decay_string (str): Decay strinng with selected particles

    Returns:
        list(list(tuple(int,str))): list of hierarchies of selected particles.
    """
    from ROOT import Belle2
    d = Belle2.DecayDescriptor()
    d.init(decay_string)
    selected_particles = []
    for _ in d.getHierarchyOfSelected():
        if len(_) > 1:
            selected_particles.append(list([tuple(__) for __ in _])[1:])
    return selected_particles


def create_daughter_aliases(list_of_variables, indices, prefix="", include_indices=True):
    """Create Aliases for all variables for a given daughter hierachy

    Arguments:
        list_of_variables (list(str)): list of variables to create aliases for
        indices (int): index of the daughter, grand-daughter, grand-grand-daughter,
            and so forth
        prefix (str): optional prefix to prepend to the aliases
        include_indices(bool): if set to True (default) the aliases will contain
            the daughter indices as dX_dY_dZ...

    * create aliases for the second daughter as "d1_E", "d1_M" (daughters start at 0)

      >>> create_daughter_aliases(["E", "m"], 1)
      ['d1_E', 'd1_m']
      >>> from variables import variables
      >>> variables.printAliases()
      [INFO] =========================
      [INFO] Following aliases exists:
      [INFO] 'd1_E' --> 'daughter(1,E)'
      [INFO] 'd1_m' --> 'daughter(1,m)'
      [INFO] =========================


    * create aliases for the first grand daughter of the second daughter,
      starting with "my" and without including the indices, resulting in "my_E", "my_m"

      >>> create_daughter_aliases(["E", "m"], [1, 0], prefix="my", include_indices=False)
      ['my_E', 'my_m']
      >>> from variables import variables
      >>> variables.printAliases()
      [INFO] =========================
      [INFO] Following aliases exists:
      [INFO] 'd1_E' --> 'daughter(1,daughter(0,E))'
      [INFO] 'd1_m' --> 'daughter(1,daughter(0,m))'
      [INFO] =========================

    * create aliases for the second grand grand daughter of the third grand
      daughter of the fifth daugther, starting with my and including the
      indices, resulting in "my_d4_d2_d1_E", "my_d4_d2_d1_m"

      >>> create_daughter_aliases(["E", "m"], [4, 2, 1], prefix="my")
      ['my_d4_d2_d1_E', 'my_d4_d2_d1_m']
      >>> from variables import variables
      >>> variables.printAliases()
      [INFO] =========================
      [INFO] Following aliases exists:
      [INFO] 'd1_E' --> 'daughter(4,daughter(2,daughter(1,E))'
      [INFO] 'd1_m' --> 'daughter(4,daughter(2,daughter(1,m))'
      [INFO] =========================

    """

    if not isinstance(indices, co.Iterable):
        indices = [indices]

    if include_indices:
        prefix = reduce(lambda x, y: f"{x}_d{y}", indices, prefix).lstrip("_")

    template = reduce(lambda x, y: f"daughter({y},{x})", reversed(indices), "{variable}")
    return create_aliases(list_of_variables, template, prefix)


def create_aliases_for_selected(list_of_variables, decay_string, prefix=""):
    """
    The function creates list of aliases for given variables so that they are calculated for
    particles selected in decay strigng.
    Aliases of variables are assigned automaticaly in the following manner:
    If namings are unambiguous, it's semi-laconic :doc:`DecayString` style:

    * pi variabels selected as ``B0 -> [D0 -> ^pi+ K-] pi0`` will have ``D0_pi_`` prefix in ntuple.:

    >>> list_of_variables = ['M','p']
    >>> decay_string = 'B0 -> [D0 -> ^pi+ K-] pi0'
    >>> print(create_aliases_for_selected(list_of_variables, decay_string))
    ['D0_pi_M', 'D0_pi_p']
    >>> from variables import variables
    >>> variables.printAliases()
    [INFO] =========================
    [INFO] Following aliases exists:
    [INFO] 'D0_pi_M' --> 'daughter(0,daughter(0,M))'
    [INFO] 'D0_pi_p' --> 'daughter(0,daughter(0,p))'
    [INFO] =========================


    If namings are ambiguous, prefix will contain hierarchy path indexes:

    * pi variabels selected as
      ``B0 -> [D0 -> ^pi+`` (1) ``^pi-`` (2) ``^pi0`` (3) ``] ^pi0`` (4)
      will have the following prefixes:

      1. ``D0_pi_00_``

      2. ``D0_pi_01_``

      3. ``D0_pi0_``

      4. ``pi0_``

    >>> list_of_variables = ['M','p']
    >>> decay_string = 'B0 -> [D0 -> ^pi+ ^pi- ^pi0] ^pi0'
    >>> print(create_aliases_for_selected(list_of_variables, decay_string))
    ['D0_pi_0_0_M', 'D0_pi_0_0_p', 'D0_pi_0_1_M', 'D0_pi_0_1_p',
     'D0_pi0_M', 'D0_pi0_p', 'pi0_M', 'pi0_p']
    >>> from variables import variables
    >>> variables.printAliases()
    [INFO] =========================
    [INFO] Following aliases exists:
    [INFO] 'D0_pi0_M' --> 'daughter(0,daughter(2,M))'
    [INFO] 'D0_pi0_p' --> 'daughter(0,daughter(2,p))'
    [INFO] 'D0_pi_0_0_M' --> 'daughter(0,daughter(0,M))'
    [INFO] 'D0_pi_0_0_p' --> 'daughter(0,daughter(0,p))'
    [INFO] 'D0_pi_0_1_M' --> 'daughter(0,daughter(1,M))'
    [INFO] 'D0_pi_0_1_p' --> 'daughter(0,daughter(1,p))'
    [INFO] 'D0_pi_M' --> 'daughter(0,daughter(0,M))'
    [INFO] 'D0_pi_p' --> 'daughter(0,daughter(0,p))'
    [INFO] 'pi0_M' --> 'daughter(1,M)'
    [INFO] 'pi0_p' --> 'daughter(1,p)'
    [INFO] =========================


    If you feel that such naming is clumsy, you call this function for each selected
    particle and add individual aliases for them:

    >>> list_of_variables = ['M','p']
    >>> decay_string = 'B0 -> [D0 -> pi+ ^pi- pi0] pi0'
    >>> aliases = create_aliases_for_selected(list_of_variables, decay_string, 'pim')
    >>> decay_string = 'B0 -> [D0 -> ^pi+ pi- pi0] pi0'
    >>> aliases += create_aliases_for_selected(list_of_variables, decay_string, 'pip')
    >>> print(aliases)
    ['pim_M', 'pim_p', 'pip_M', 'pip_p']
    >>> from variables import variables
    >>> variables.printAliases()
    [INFO] =========================
    [INFO] Following aliases exists:
    [INFO] 'pim_M' --> 'daughter(0,daughter(1,M))'
    [INFO] 'pim_p' --> 'daughter(0,daughter(1,p))'
    [INFO] 'pip_M' --> 'daughter(0,daughter(0,M))'
    [INFO] 'pip_p' --> 'daughter(0,daughter(0,p))'
    [INFO] =========================

    Parameters:
        list_of_variables (list(str)): list of variable names
        decay_string (str): Decay strinng with selected particles

    Returns:
        list(str): new variables list
    """
    if decay_string.count("^") == 1:
        selected_particles = [x[0] for x in get_hierarchy_of_decay(decay_string)[0]]
        return create_daughter_aliases(list_of_variables, selected_particles, prefix)

    selected_particles = get_hierarchy_of_decay(decay_string)
    prefixes = {}
    unique_namings = []

    for particle in selected_particles:
        name_prefix = "_".join([x[1] for x in particle])
        hierarchy_path = [x[0] for x in particle]
        if name_prefix in unique_namings:
            if name_prefix in prefixes.keys():
                hp = prefixes[name_prefix]
                unambiguous_prefix = name_prefix + "_" + "_".join([str(x) for x in hp])
                prefixes[unambiguous_prefix] = hp
                del prefixes[name_prefix]
            unambiguous_prefix = name_prefix + "_" + "_".join([str(x) for x in hierarchy_path])
            prefixes[unambiguous_prefix] = hierarchy_path
            continue
        else:
            unique_namings.append(name_prefix)
            prefixes[name_prefix] = hierarchy_path
    var_list = []
    for p in prefixes.keys():
        var_list += create_daughter_aliases(list_of_variables, prefixes[p], prefix + p)

    return var_list


def make_mc(list_of_variables):
    """
    The function wraps variables from the list with 'matchedMC()'.

    >>> list_of_variables = ['M','p']
    >>> print(make_mc(list_of_variables))
    ['mmc_M', 'mmc_p']
    >>> from variables import variables
    >>> variables.printAliases()
    [INFO] =========================
    [INFO] Following aliases exists:
    [INFO] 'mmc_M' --> 'matchedMC(M)'
    [INFO] 'mmc_p' --> 'matchedMC(p)'
    [INFO] =========================


    Parameters:
        list_of_variables (list(str)): list of variable names

    Returns:
        list(str): new variables list
    """
    return create_aliases(list_of_variables,
                          'matchedMC({variable})',
                          'mmc')


def add_collection(list_of_variables, collection_name):
    """
    The function creates variable collection from tne list of variables

    Note: This is kept for compatibility.

    Parameters:
        list_of_variables (list(str)): list of variable names
        collection_name (str): name of the collection


    Returns:
        str: name of the variable collection
    """
    import variables as v
    v.variables.addCollection(collection_name, v.std_vector(*tuple(list_of_variables)))
    return collection_name
