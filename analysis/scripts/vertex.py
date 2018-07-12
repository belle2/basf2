#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import os
import sys
import inspect
from analysisPath import *


def fitVertex(
    list_name,
    conf_level,
    decay_string='',
    fitter='rave',
    fit_type='vertex',
    constraint='',
    daughtersUpdate=False,
    path=analysis_main,
):
    """
    Perform the specified kinematic fit for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit. 0 selects CL > 0
    @param decay_string select particles used for the vertex fit
    @param fitter       rave or kfitter
    @param fit_type     type of the kinematic fit (valid options are vertex/massvertex/mass)
    @param constraint   type of additional constraints (valid options are empty string/ipprofile/iptube/mother)
    @param updateDaughters make copy of the daughters and update them after the vertex fit
    @param path         modules are added to this path
    """

    pvfit = register_module('ParticleVertexFitter')
    pvfit.set_name('ParticleVertexFitter_' + list_name)
    pvfit.param('listName', list_name)
    pvfit.param('confidenceLevel', conf_level)
    pvfit.param('vertexFitter', fitter)
    pvfit.param('fitType', fit_type)
    pvfit.param('withConstraint', constraint)
    pvfit.param('updateDaughters', daughtersUpdate)
    pvfit.param('decayString', decay_string)
    path.add_module(pvfit)


def vertexKFit(
    list_name,
    conf_level,
    decay_string='',
    constraint='',
    path=analysis_main,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param constraint   add aditional constraint to the fit (valid options are ipprofile or iptube)
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'kfitter',
        'vertex',
        constraint,
        False,
        path,
    )


def vertexKFitDaughtersUpdate(
    list_name,
    conf_level,
    constraint='',
    path=analysis_main,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList and update the Daughters.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param constraint   add aditional constraint to the fit (valid options are ipprofile or iptube)
    @param path         modules are added to this path
    """

    fitVertex(
        list_name,
        conf_level,
        '',
        'kfitter',
        'vertex',
        constraint,
        True,
        path,
    )


def massVertexKFit(
    list_name,
    conf_level,
    decay_string='',
    path=analysis_main,
):
    """
    Perform mass-constrained vertex fit using the kfitter for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'kfitter',
        'massvertex',
        '',
        False,
        path,
    )


def massVertexKFitDaughtersUpdate(
    list_name,
    conf_level,
    decay_string='',
    path=analysis_main,
):
    """
    Perform mass-constrained vertex fit using the kfitter for each Particle in the given ParticleList and update the daughters.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'kfitter',
        'massvertex',
        '',
        True,
        path,
    )


def massKFit(
    list_name,
    conf_level,
    decay_string='',
    path=analysis_main,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'kfitter',
        'mass',
        '',
        False,
        path,
    )


def massKFitDaughtersUpdate(
    list_name,
    conf_level,
    decay_string='',
    path=analysis_main,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList and update the daughters.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'kfitter',
        'mass',
        '',
        True,
        path,
    )


def fourCKFit(
    list_name,
    conf_level,
    decay_string='',
    path=analysis_main,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'kfitter',
        'fourC',
        '',
        False,
        path,
    )


def fourCKFitDaughtersUpdate(
    list_name,
    conf_level,
    decay_string='',
    path=analysis_main,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList and update the daughters.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'kfitter',
        'fourC',
        '',
        True,
        path,
    )


def vertexRave(
    list_name,
    conf_level,
    decay_string='',
    constraint='',
    path=analysis_main,
):
    """
    Perform vertex fit using the RAVE for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param constraint   add aditional constraint to the fit (valid options are ipprofile or iptube)
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'rave',
        'vertex',
        constraint,
        False,
        path,
    )


def vertexRaveDaughtersUpdate(
    list_name,
    conf_level,
    decay_string='',
    constraint='',
    path=analysis_main,
):
    """
    Perform vertex fit using RAVE for each Particle in the given ParticleList and update the Daughters.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param decay_string select particles used for the vertex fit
    @param constraint   add aditional constraint to the fit (valid options are ipprofile or iptube)
    @param path         modules are added to this path
    """

    puvfit = register_module('VertexFitUpdateDaughters')
    puvfit.set_name('VertexFitUpdateDaughters_' + list_name)
    puvfit.param('listName', list_name)
    puvfit.param('confidenceLevel', conf_level)
    puvfit.param('decayString', decay_string)
    puvfit.param('withConstraint', constraint)
    path.add_module(puvfit)


def massVertexRave(
    list_name,
    conf_level,
    decay_string='',
    path=analysis_main,
):
    """
    Perform mass-constrained vertex fit using the RAVE for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'rave',
        'massvertex',
        '',
        False,
        path,
    )


def massVertexRaveDaughtersUpdate(
    list_name,
    conf_level,
    decay_string='',
    path=analysis_main,
):
    """
    Perform mass-constrained vertex fit using the RAVE for each Particle in the given ParticleList and update the daughters.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'rave',
        'massvertex',
        '',
        True,
        path,
    )


def massRave(
    list_name,
    conf_level,
    decay_string='',
    path=analysis_main,
):
    """
    Perform mass fit using the RAVE for each Particle in the given ParticleList.
    7x7 error matrix of the mother particle must be defined

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'rave',
        'mass',
        '',
        False,
        path,
    )


def vertexTree(
    list_name,
    conf_level=0.001,
    massConstraint=[],
    ipConstraint=False,
    updateAllDaughters=False,
    customOriginConstraint=False,
    customOriginVertex=[0.001, 0, 0.0116],
    customOriginCovariance=[0.0048, 0, 0, 0, 0.003567, 0, 0, 0, 0.0400],
    path=analysis_main,
):
    """
    Perform the specified kinematic fit for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit. 0 selects CL > 0
    @param massConstraint list of PDG ids which are mass-constrained
    @param ipConstraint constrain head production vertex to IP (x-y-z) constraint, default: False)
    @param customOriginConstraint use a costum origin vertex as the production vertex of your particle." + \
        "This is usefull when fitting D*/D without wanting to fit a B but constraining the process to be B-decay like + \
        "(think of semileptonic modes and stuff with a neutrino in the B decay). Default: False"
    @param customOriginVertex 3d vector of the vertex coordinates you want to use as custom origin."+\
        "Default numbers are taken for B-mesons
    @param customOriginCovariance 3x3 covariance matrix for the custom vertex (type: vector)." +\
        " Default numbers extracted from generator distribtuion width of B-mesons.
    @param updateAllDaughters if true the entire tree will be updated with the fitted values. Otherwise only the head.
    @param path         modules are added to this path
    """

    treeFitter = register_module("TreeFitter")
    treeFitter.set_name('TreeFitter_' + list_name)
    treeFitter.param('particleList', list_name)
    treeFitter.param('confidenceLevel', conf_level)
    treeFitter.param('massConstraintList', massConstraint)
    treeFitter.param('ipConstraint', ipConstraint)
    treeFitter.param('updateAllDaughters', updateAllDaughters)
    treeFitter.param('customOriginConstraint', customOriginConstraint)
    treeFitter.param('customOriginVertex', customOriginVertex)
    treeFitter.param('customOriginCovariance', customOriginCovariance)

    path.add_module(treeFitter)


def TagV(
    list_name,
    MCassociation='',
    confidenceLevel=0.,
    useFitAlgorithm='standard_PXD',
    askMCInfo=False,
    reqPXDHits=0,
    path=analysis_main,
):
    """
    For each Particle in the given Breco ParticleList:
    perform the fit of tag side using the track list from the RestOfEvent dataobject
    save the MC Btag in case of signal MC

    @param list_name name of the input Breco ParticleList
    @param confidenceLevel minimum value of the ConfidenceLevel to accept the fit. 0 selects CL > 0
    @param MCassociation: use standard MC association or the internal one
    @param useConstraint: choose constraint for the tag vertes fit
    @param reqPXDHits: minimum N PXD hits for a track
    @param path      modules are added to this path
    """

    tvfit = register_module('TagVertex')
    tvfit.set_name('TagVertex_' + list_name)
    tvfit.param('listName', list_name)
    tvfit.param('confidenceLevel', confidenceLevel)
    tvfit.param('MCAssociation', MCassociation)
    tvfit.param('useFitAlgorithm', useFitAlgorithm)
    tvfit.param('askMCInformation', askMCInfo)
    tvfit.param('reqPXDHits', reqPXDHits)
    path.add_module(tvfit)


if __name__ == '__main__':
    desc_list = []
    for function_name in sorted(list_functions(sys.modules[__name__])):
        function = globals()[function_name]
        signature = inspect.formatargspec(*inspect.getargspec(function))
        signature = signature.replace(repr(analysis_main), 'analysis_main')
        desc_list.append((function.__name__, signature + '\n' + function.__doc__))
    pretty_print_description_list(desc_list)
