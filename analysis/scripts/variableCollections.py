#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Adds default collections to the VariableManager
Details can be found on https://confluence.desy.de/display/BI/Physics+VariableManager

"""

# from variables import variables

# # TODO add everything from analysis/NtupleTools/src/NtupleToolList.cc

# variables.addCollection("Kinematics", v.std_vector("M", "p", "px", "py", "pz", "E"))
# variables.addCollection("MCTruth", v.std_vector("isSignal", "mcErrors"))


def get_hierarchy_of_decay(decay_string):
    """
    This function returns hierarchy pathes of the particles selected in decay string.
    Hierarchy path is vector of pairs of relative daughter numbers and particle names.
    For instance, in decay
    B+ -> [ D+ -> ^K+ pi0 ] pi0
    decay path of K+ is
    [(0, B), (0, D), (0 K)]
    Every selected partcile has its own hierarchy path and
    they are stored as a vector in this variable:
    For the decayString
    B+ -> [ D+ -> ^K+ pi0 ] ^pi0
    m_hierarchy, once filled, is
    [[(0, D), (0, K)],
    [(1, pi0)]]

    Note: hierarchy path here is slightly different from one in DecayDescriptor class.
    The difference is that the first element in all pathes (common mother particle) is ommited

    Parameters:
        decay_string (str): Decay strinng with selected particles
    """
    from ROOT import gSystem
    gSystem.Load('libanalysis.so')
    from ROOT import Belle2
    d = Belle2.DecayDescriptor.Instance()
    d.init(decay_string)
    selected_particles = []
    for _ in d.getHierarchyOfSelected():
        if len(_) > 1:
            selected_particles.append(list([tuple(__) for __ in _])[1:])
    d.destroy()
    return selected_particles


def convert_to_daughter_vars(variables_list, decay_string):
    """
    The function transforms list of variables to that for
    particles selected in decay strigng.
    Aliases of variables are assigned automaticaly.

    Parameters:
        variables_list (list(str)): list of variable names
        decay_string (str): Decay strinng with selected particles
    """
    selected_particles = get_hierarchy_of_decay(decay_string)
    prefixes = {}
    ambiguous_namings = []
    ambiguous_namings_and_hierarchies = {}

    for particle in selected_particles:
        name_prefix = "_".join([x[1] for x in particle])
        hierarchy_path = [x[0] for x in particle]
        if name_prefix in ambiguous_namings:
            ambiguous_namings_and_hierarchies[hierarchy_path] = name_prefix
            continue
        if name_prefix not in prefixes.keys():
            prefixes[name_prefix] = hierarchy_path
        else:
            ambiguous_namings.append(name_prefix)
            ambiguous_namings_and_hierarchies[hierarchy_path] = name_prefix
            ambiguous_namings_and_hierarchies[prefixes[hierarchy_path]] = name_prefix
            del prefixes[name_prefix]

    for _ in ambiguous_namings_and_hierarchies.keys():
        h_prefix = "".join()
        prefixes[ambiguous_namings_and_hierarchies[_] + "_" + "".join(_)] = _

    var_list = []
    for p in prefixes.keys():
        var_list += convert_to_nd_vars(variables_list, prefixes[p], p)

    return var_list


def convert_to_daughter_vars(variables_list, decay_string, alias_prefix):
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
    """
    if decay_string.count("^") != 1:
        B2FATAL("Please use only one '^' per call of the function")
    selected_particles = [x[0] for x in get_hierarchy_of_decay(decay_string)[0]]
    return convert_to_nd_vars(variables_list, hierarchy_path, alias_prefix)


def convert_to_nd_vars(variables_list, hierarchy_path, alias_prefix):
    """
    The function transforms list of variables to that relative for the particle's n-th daughter.
    Daughter numbering starts from 0, daughters are ordered by mass.
    Daughters with the same mass are ordred by charge.

    Parameters:
        variables_list (list(str)): list of variable names
        hierarchy_path (list(int)): hierarchy path (sequence of numbers of daughters that brings to the paericle)
        alias_prefix (str): User-defined alias prefix for trannsformed list
    """
    wrapper = "variable"
    for h in reversed(hierarchy_path):
        wrapper = "daughter(" + str(h) + "," + wrapper + ")"
    return wrap_list(variables_list, wrapper, alias_prefix)


def wrap_list(variables_list, wrapper, alias_prefix):
    """
    The function wraps every variable from variables list with given wrapper
    and add alias using given alias prefix

    Parameters:
        variables_list (list(str)): list of variable names
        wrapper (str): wrapper in format `<wrapper_name>(<some configs>,variable,<some configs)`
        alias_prefix (str): alias prefix used for wrapped variables.
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


def convert_to_daughter_vars(variables_list, daughter_number):
    """
    The function transforms list of variables to that for the n-th daughter.
    Daughter numbering starts from 0, daughters are ordered by mass.
    Daughters with the same mass are ordred by charge.

    Parameters:
        variables_list (list(str)): list of variable names
        daugther_numer (int): serial number of a daughter (starting from 0)
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
    """
    return wrap_list(variables_list,
                     'daughter(' + str(daughter_number) + ',daughter(' + str(granddaughter_number) + ',variable))',
                     'd' + str(daughter_number) + '_d' + str(granddaughter_number))


def make_mc(variables_list):
    """
    The function wraps variables from the list with 'matchedMC()'.

    Parameters:
        variables_list (list(str)): list of variable names
    """
    return wrap_list(variables_list,
                     'matchedMC(variable)',
                     'mmc')


def add_collection(variables_list, collection_name):
    """
    The function creates variable collection from tne list of variables

    Parameters:
        variables_list (list(str)): list of variable names
        collection_name (str): name of the collection
    """
    from variables import variables
    variables.addCollection(collection_name, v.std_vector(tuple(variables_list)))
    return collection_name


# Replacement for DeltaEMbc
mbc_deltae_variables = [
    "Mbc",
    "deltaE"]

# Event variables
event_variables = [
    'evtNum',
    'expNum',
    'productionIdentifier',
    'runNum']

# Kinematic variables
kinematic_variables = ['px',
                       'py',
                       'pz',
                       'pt',
                       'p',
                       'E',
                       'M',
                       'ErrM',
                       'SigM',
                       'InvM']

# Kinematic variables in CMS
CMS_kinematic_variables = wrap_list(kinematic_variabels,
                                    "useCMSFrame(variable)",
                                    "CMS_")

# Cluster-related variables
cluster_variables = [
    'clusterE',
    'clusterReg']

# Tracking variables
track_variables = [
    'dr',
    'dx',
    'dy',
    'dz',
    'd0',
    'z0',
    'nCDCHits',
    'nPXDHits',
    'nSVDHits',
    'nVXDHits',
    'pValue']

# Truth-matching related variables
mc_variables = [
    'genMotherID',
    'genMotherP',
    'genMotherPDG',
    'genParticleID',
    'isCloneTrack',
    'mcDX',
    'mcDY',
    'mcDZ',
    'mcDecayTime',
    'mcE',
    'mcErrors',
    'mcInitial',
    'mcP',
    'mcPDG',
    'mcPT',
    'mcPX',
    'mcPY',
    'mcPZ',
    'mcPhi',
    'mcVirtual',
    'nMCMatches']

# PID variables
pid_variables = [
    'kaonID',
    'pionID',
    'protonID',
    'muonID',
    'electronID']

# Replacement for ROEMultiplicities tool
roe_multiplicities = [
    'nROEKLMClusters']

# Recoil kinematics relaed variables
recoil_kinematics = [
    'pRecoil',
    'pRecoilPhi',
    'pRecoilTheta',
    'pxRecoil',
    'pyRecoil',
    'pzRecoil']

# see BII-3874
flight_info = []

# see BII-3874
mc_flight_info = []

# see BII-3876
vertex = [
    'x',
    'x_uncertainty',
    'y',
    'y_uncertainty',
    'z',
    'z_uncertainty',
    'pValue']

# see BII-3876
mc_vertex = [
    'matchedMC(x)',
    'matchedMC(x_uncertainty)',
    'matchedMC(y)',
    'matchedMC(y_uncertainty)',
    'matchedMC(z)',
    'matchedMC(z_uncertainty)',
    'matchedMC(pValue)']

# Tag-side related variables
tag_vertex = [
    'TagVLBoost',
    'TagVLBoostErr',
    'TagVOBoost',
    'TagVOBoostErr',
    'TagVpVal',
    'TagVx',
    'TagVxErr',
    'TagVy',
    'TagVyErr',
    'TagVz',
    'TagVzErr',
]

# Tag-side  related MC true variables
mc_tag_vertex = [
    'MCDeltaT',
    'MCTagBFlavor',
    'TagVmcLBoost',
    'TagVmcOBoost',
    'mcLBoost',
    'mcOBoost',
    'mcTagVx',
    'mcTagVy',
    'mcTagVz',
]

# Replacement for MomentumUnertainty tool
momentum_uncertainty = [
    'E_uncertainty',
    'pxErr',
    'pyErr',
    'pzErr']
