#!/usr/bin/env python3
# -*- coding: utf-8 -*-


def wrap_list(variables_list, wrapper, alias_prefix):
    """
    The function wraps every variable from variables list with given wrapper
    and add alias using given alias prefix

    >>> variables_list = ['M','p']
    >>> wrapper = 'daughter(1,{variable})'
    >>> alias_prefix = 'pref'
    >>> print(wrap_list(variables_list, wrapper, alias_prefix))
    ['pref_M', 'pref_p']

    Parameters:
        variables_list (list(str)): list of variable names
        wrapper (str): metafunction taking variables from variables_list as a parameter
        (``<metafunction>(<some configs>, {variable} ,<some otehr configs>)``
        alias_prefix (str): alias prefix used for wrapped variables.

    Returns:
        list(str): new variables list
    """
    from variables import variables
    aliases = [f"{alias_prefix}_{e}" for e in variables_list]
    for var, alias in zip(variables_list, aliases):
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


def convert_to_all_selected_vars(variables_list, decay_string):
    """
    The function transforms list of variables to that for
    particles selected in decay strigng.
    Aliases of variables are assigned automaticaly in the following manner:
    If namings are unambiguous, it's semi-laconic :doc:`DecayString` style:

    * pi variabels selected as ``B0 -> [D0 -> ^pi+ K-] pi0`` will have ``D0_pi_`` prefix in ntuple.:

    >>> variables_list = ['M','p']
    >>> decay_string = 'B0 -> [D0 -> ^pi+ K-] pi0'
    >>> print(convert_to_all_selected_vars(variables_list, decay_string))
    ['D0_pi_M', 'D0_pi_p']

    If namings are ambiguous, prefix will contain hierarchy path indexes:

    * pi variabels selected as
      ``B0 -> [D0 -> ^pi+`` (1) `` ^pi-`` (2) `` ^pi0`` (3) `` ] ^pi0`` (4)
      will have the following prefixes:

      1. ``D0_pi_00_``

      2. ``D0_pi_01_``

      3. ``D0_pi0_``

      4. ``pi0_``

    >>> variables_list = ['M','p']
    >>> decay_string = 'B0 -> [D0 -> ^pi+ ^pi- ^pi0] ^pi0'
    >>> print(convert_to_all_selected_vars(variables_list, decay_string))
    ['D0_pi_0_0_M', 'D0_pi_0_0_p', 'D0_pi_0_1_M', 'D0_pi_0_1_p',
     'D0_pi0_M', 'D0_pi0_p', 'pi0_M', 'pi0_p']

    If you feel that such naming is clumsy, you always can add individual aliases
    for particles with `convert_to_one_selected_vars()` function:
    >>> variables_list = ['M','p']
    >>> decay_string = 'B0 -> [D0 -> pi+ ^pi- pi0] pi0'
    >>> aliases = convert_to_one_selected_vars(variables_list, decay_string, 'pim')
    >>> decay_string = 'B0 -> [D0 -> ^pi+ pi- pi0] pi0'
    >>> aliases += convert_to_one_selected_vars(variables_list, decay_string, 'pip')
    >>> print(aliases)
    ['pim_M', 'pim_p', 'pip_M', 'pip_p']

    Parameters:
        variables_list (list(str)): list of variable names
        decay_string (str): Decay strinng with selected particles

    Returns:
        list(str): new variables list
    """
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
        var_list += convert_to_nd_vars(variables_list, prefixes[p], p)

    return var_list, prefixes


def convert_to_one_selected_vars(variables_list, decay_string, alias_prefix):
    """
    The function transforms list of variables to that for the
    particle selected in decay string.

    Note 1: Only one particle can be selected in the DcayString.
    If you want to apply variables for several particles, either use
    this function without specifyng alias prefix or call the function
    for each child particle independently.

    >>> variables_list = ['M','p']
    >>> decay_string = 'B0 -> [D0 -> pi+ ^pi- pi0] pi0'
    >>> print(convert_to_one_selected_vars(variables_list, decay_string, 'pim'))
    ['pim_M', 'pim_p']

    Parameters:
        variables_list (list(str)): list of variable names
        decay_string (str): Decay strinng with selected particle
        alias_prefix (str): User-defined alias prefix for trannsformed list

    Returns:
        list(str): new variables list
    """
    if decay_string.count("^") != 1:
        B2FATAL("Please use only one '^' per call of the function")
    selected_particles = [x[0] for x in get_hierarchy_of_decay(decay_string)[0]]
    return convert_to_nd_vars(variables_list, selected_particles, alias_prefix)


def convert_to_nd_vars(variables_list, hierarchy_path, alias_prefix):
    """
    The function transforms list of variables to that relative for the one of the particle's daughter.
    This is helper function used by other functions in this package since it requires
    quite peculiar input, namely ``hierarchy_path``, that is sequence od number of daughters
    ending with the number of selected particle. For instance, ``hierarchy_path`` of
    ``pi-`` in ``B0 -> [D0 -> pi+ ^pi- pi0] pi0`` is ``[0, 1]`` and of
    ``pi0`` in ``B0 -> [D0 -> pi+ pi- pi0] ^pi0`` is ``[1]``

    >>> variables_list = ['M','p']
    >>> # Users don't use the finction, so let's define hierarcy path
    >>> # for decay 'B0 -> [D0 -> pi+ ^pi- pi0] pi0' by hands:
    >>> hierarcy_path = [0, 1]
    >>> print(convert_to_nd_vars(variables_list, hierarcy_path, 'pim'))
    ['pim_M', 'pim_p']

    Parameters:
        variables_list (list(str)): list of variable names
        hierarchy_path (list(int)): hierarchy path (sequence of numbers of daughters that brings to the particle)
        alias_prefix (str): User-defined alias prefix for trannsformed list

    Returns:
        list(str): new variables list
    """
    wrapper = '{{variable}}'
    for h in reversed(hierarchy_path):
        wrapper = "daughter(" + str(h) + "," + wrapper + ")"
    return wrap_list(variables_list, wrapper, alias_prefix)


def convert_to_daughter_vars(variables_list, daughter_number):
    """
    The function transforms list of variables to that for the n-th daughter.

    >>> variables_list = ['M','p']
    >>> daughter_number = 1
    >>> print(convert_to_daughter_vars(variables_list, daughter_number))
    ['d1_M', 'd1_p']


    Parameters:
        variables_list (list(str)): list of variable names
        daugther_numer (int): serial number of a daughter (starting from 0)

    Returns:
        list(str): new variables list
    """
    return wrap_list(variables_list,
                     "daughter(" + str(daughter_number) + ",{{variable}})",
                     "d" + str(daughter_number))


def convert_to_gd_vars(variables_list, daughter_number, granddaughter_number):
    """
    The function transforms list of variables to that relative for the particle's granddaughter.

    >>> variables_list = ['M','p']
    >>> daughter_number = 1
    >>> granddaughter_number = 2
    >>> print(convert_to_gd_vars(variables_list, daughter_number, granddaughter_number))
    ['d1_d2_M', 'd1_d2_p']

    Parameters:
        variables_list (list(str)): list of variable names
        daugther_numer (int): serial number of a daughter (starting from 0)
        granddaughter_number (int): serial number of a daughter's daughter (starting from 0)

    Returns:
        list(str): new variables list
    """
    return wrap_list(variables_list,
                     f"daughter({daughter_number}, daughter({granddaughter_number}, {{variable}}))",
                     f"d{daughter_number}_d{granddaughter_number}")


def make_mc(variables_list):
    """
    The function wraps variables from the list with 'matchedMC()'.

    >>> variables_list = ['M','p']
    >>> print(make_mc(variables_list))
    ['mmc_M', 'mmc_p']

    Parameters:
        variables_list (list(str)): list of variable names

    Returns:
        list(str): new variables list
    """
    return wrap_list(variables_list,
                     'matchedMC({{variable}})',
                     'mmc')


def add_collection(variables_list, collection_name):
    """
    The function creates variable collection from tne list of variables

    Note: This is kept for compatibility.

    Parameters:
        variables_list (list(str)): list of variable names
        collection_name (str): name of the collection


    Returns:
        str: name of the variable collection
    """
    from variables import variables
    variables.addCollection(collection_name, v.std_vector(tuple(variables_list)))
    return collection_name
