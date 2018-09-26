#!/usr/bin/env python3
# -*- coding: utf-8 -*-


def wrap_list(variables_list, wrapper, alias_prefix):
    """
    The function wraps every variable from variables list with given wrapper
    and add alias using given alias prefix

    Parameters:
        variables_list (list(str)): list of variable names
        wrapper (str): wrapper in format ``<wrapper_name>(<some configs>,variable,<some configs)``
        alias_prefix (str): alias prefix used for wrapped variables.

    Returns:
        list(str): new variables list
    """
    wrapped_list = []
    from variables import variables

    import re
    count = sum(1 for _ in re.finditer(r'\b%s\b' % re.escape('varieble'), wrapper))
    if count != 0:
        B2FATAL("Can't parse wrapper : " + wrapper +
                " Please make sure it has format `<wrapper_name>(<some configs>,variable,<some configs>)`")

    left_side_of_wrapper = wrapper.split("variable")[0]
    right_side_of_wrapper = wrapper.split("variable")[1]
    for v in variables_list:
        alias = alias_prefix + "_" + v
        variables.addAlias(alias, left_side_of_wrapper + v + right_side_of_wrapper)
        wrapped_list.append(alias)
    return wrapped_list


def get_hierarchy_of_decay(decay_string):
    """
    This function returns hierarchy pathes of the particles selected in decay string.
    In C++ part of the basf2, hierarchy path is vector of pairs of relative daughter numbers and particle names.
    For instance, in decay
    ``B+ -> [ D+ -> ^K+ pi0 ] pi0``
    decay path of K+ is
    ``[(0, B), (0, D), (0 K)]``
    Every selected partcile has its own hierarchy path and
    they are stored as a vector in this variable:
    For the decayString
    ``B+ -> [ D+ -> ^K+ pi0 ] ^pi0``
    m_hierarchy, once filled, is
    ``[[(0, D), (0, K)],
    [(1, pi0)]]``

    Note: here C++ object are converted to python ones (vector of pairs becomes list of tuples)

    Note: hierarchy path here is slightly different from one in DecayDescriptor class.
    The difference is that the first element in all pathes (common mother particle) is ommited

    Parameters:
        decay_string (str): Decay strinng with selected particles

    Returns:
        list(list(tuple(int,str))): list of hierarchies of selected particles.
    """
    from ROOT import Belle2
    d = Belle2.DecayDescriptor.Instance()
    d.init(decay_string)
    selected_particles = []
    for _ in d.getHierarchyOfSelected():
        if len(_) > 1:
            selected_particles.append(list([tuple(__) for __ in _])[1:])
    print(selected_particles)
    d.drop()
    return selected_particles


def convert_to_all_selected_vars(variables_list, decay_string):
    """
    The function transforms list of variables to that for
    particles selected in decay strigng.
    Aliases of variables are assigned automaticaly in the following manner:
    If namings are unambiguous, it's semi-laconic :doc:`DecayString` style:

    * pi variabels selected as ``B0 -> [D0 -> ^pi+ K-] pi0`` will have ``D0_pi_`` prefix in ntuple.

    If namings are ambiguous, prefix will contain hierarchy path indexes:

    * pi variabels selected as
      ``B0 -> [D0 -> ^pi+`` (1) `` ^pi-`` (2) `` ^pi0`` (3) `` ] ^pi0`` (4)
      will have the following prefixes:

      1. ``D0_pi_00_``

      2. ``D0_pi_01_``

      3. ``D0_pi0_``

      4. ``pi0_``

    If you feel that such naming is clumsy, you always can add individual aliases
    for particles with `convert_to_all_selected_vars()` function.

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

    return var_list


def convert_to_one_selected_vars(variables_list, decay_string, alias_prefix):
    """
    The function transforms list of variables to that for the
    particle selected in decay string.

    Note 1: Only one particle can be selected in the DcayString.
    If you want to apply variables for several particles, either use
    this function without specifyng alias prefix or call the function
    for each child particle independently.

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
    The function transforms list of variables to that relative for the particle's n-th daughter.
    Daughter numbering starts from 0, daughters are ordered by mass.
    Daughters with the same mass are ordred by charge.

    Parameters:
        variables_list (list(str)): list of variable names
        hierarchy_path (list(int)): hierarchy path (sequence of numbers of daughters that brings to the paericle)
        alias_prefix (str): User-defined alias prefix for trannsformed list


    Returns:
        list(str): new variables list
    """
    wrapper = "variable"
    for h in reversed(hierarchy_path):
        wrapper = "daughter(" + str(h) + "," + wrapper + ")"
    return wrap_list(variables_list, wrapper, alias_prefix)


def convert_to_daughter_vars(variables_list, daughter_number):
    """
    The function transforms list of variables to that for the n-th daughter.
    Daughter numbering starts from 0, daughters are ordered by mass.
    Daughters with the same mass are ordred by charge.

    Parameters:
        variables_list (list(str)): list of variable names
        daugther_numer (int): serial number of a daughter (starting from 0)

    Returns:
        list(str): new variables list
    """
    return wrap_list(variables_list,
                     "daughter(" + str(daughter_number) + ",variable)",
                     "d" + str(daughter_number))


def convert_to_gd_vars(variables_list, daughter_number, granddaughter_number):
    """
    The function transforms list of variables to that relative for the particle's granddaughter.
    Daughter numbering starts from 0, daughters are ordered by mass.
    Daughters with the same mass are ordred by charge.

    Parameters:
        variables_list (list(str)): list of variable names
        daugther_numer (int): serial number of a daughter (starting from 0)
        granddaughter_number (int): serial number of a daughter's daughter (starting from 0)

    Returns:
        list(str): new variables list
    """
    return wrap_list(variables_list,
                     'daughter(' + str(daughter_number) + ',daughter(' + str(granddaughter_number) + ',variable))',
                     'd' + str(daughter_number) + '_d' + str(granddaughter_number))


def make_mc(variables_list):
    """
    The function wraps variables from the list with 'matchedMC()'.

    Parameters:
        variables_list (list(str)): list of variable names


    Returns:
        list(str): new variables list
    """
    return wrap_list(variables_list,
                     'matchedMC(variable)',
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
