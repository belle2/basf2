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
    If namings are unambiguous, it's semi-laconic DecayString style:
        pi variabels selected as
            B0 -> [D0 -> ^pi+ K-] pi0
        will have
            D0_pi_
        prefix in ntuple
    If namings are ambiguous, prefix will contain hierarchy path indexes
        pi variabels selected as
            B0 -> [D0 -> ^pi+ ^pi- ^pi0 ] ^pi0
                            1.   2.   3.     4.
            will have the following prefixes:
                1: D0_pi_00_
                2: D0_pi_01_
                3: D0_pi0_
                4: pi0_

    If you feel that such naminf is clumsy, you always can add individual aliases
    for particles with `convert_to_all_selected_vars()` function.

    Parameters:
        variables_list (list(str)): list of variable names
        decay_string (str): Decay strinng with selected particles
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
deltae_mbc = [
    "Mbc",
    "deltaE"]

# Event variables
# (Replacement for EventMetaData tool)
event_meta_data = [
    'evtNum',
    'expNum',
    'productionIdentifier',
    'runNum']

# Replacement to Kinematics tool
kinematics = ['px',
              'py',
              'pz',
              'pt',
              'p',
              'E']

# Kinematic variables in CMS
ckm_kinematics = wrap_list(kinematics,
                           "useCMSFrame(variable)",
                           "CMS")

# Cluster-related variables
# Many thinds needs to be added to replace Cluster Tool
# BII-3896
cluster = [
    'clusterE',
    'clusterReg']

# Tracking variables
# Replacement for Track tool
track = [
    'dr',
    'dx',
    'dy',
    'dz',
    'd0',
    'z0',
    'pValue']

# Replacement for TrackHits tool
track_hits = [
    'nCDCHits',
    'nPXDHits',
    'nSVDHits',
    'nVXDHits']

# Replacement for MCTruth tool
mc_truth = [
    'mcErrors',
    'mcPDG']

# Replacement for MCKinematics tool
mc_kinematics = [
    'mcE',
    'mcP',
    'mcPT',
    'mcPX',
    'mcPY',
    'mcPZ',
    'mcPhi'
]

# Replacement for MCHierarchy tool
# What's missing: grandmother and grand-grand-mother ID
# [BII-3870]
mc_hierarchy = [
    'genMotherID']

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
pid = [
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

# Replacement for Vertex tuple tool
# see BII-3876
vertex = [
    'x',
    'x_uncertainty',
    'y',
    'y_uncertainty',
    'z',
    'z_uncertainty',
    'pValue']

# Replacement for MVVertex tuple tool
# see BII-3876
mc_vertex = make_mc(vertex)

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

# Replacement for RecoStats tool
# These variables are missing in variable manager
# 'nChargedECLClusters',
# 'nNeutralECLClusters',
# 'neutralECLEnergy',
# 'chargedECLEnergy',
# 'nMCParticles',
# 'nParticles'
#  [BII-3759]
reco_stats = [
    'nECLClusters',
    'nTracks',
]

# Replacement for InvMass tool
inv_mass = [
    'M',
    'ErrM',
    'SigM',
    'InvM'
]

# Replacement for MassBeforeFit tool
mass_before_fit = [
    'ErrM',
    'InvM'
]
