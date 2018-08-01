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


mbc_deltae_variables = [
    "Mbc",
    "deltaE"]

event_variables = [
    'evtNum',
    'expNum',
    'productionIdentifier',
    'runNum']

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

CMS_kinematic_variables = wrap_list(kinematic_variabels,
                                    "useCMSFrame(variable)",
                                    "CMS_")


cluster_variables = [
    'clusterE',
    'clusterReg']

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

pid_variables = [
    'kaonID',
    'pionID',
    'protonID',
    'muonID',
    'electronID']

roe_multiplicities = [
    'nROEKLMClusters']

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

momentum_uncertainty = [
    'E_uncertainty',
    'pxErr',
    'pyErr',
    'pzErr']
