#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import register_module
from basf2 import B2WARNING
import sys
import inspect
from analysisPath import analysis_main


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
    Wrapper function to provide a deprecation warning.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit. 0 selects CL > 0
    @param decay_string select particles used for the vertex fit
    @param fitter       rave or kfitter
    @param fit_type     type of the kinematic fit (valid options are vertex/massvertex/mass)
    @param constraint   type of additional constraints (valid options are empty string/ipprofile/iptube/mother)
    @param daughtersUpdate make copy of the daughters and update them after the vertex fit
    @param path         modules are added to this path
    """

    warning = (
        "Direct use of fitVertex is deprecated.\n"
        "Please use vertexKFit, vertexRave, or any of the other convenience functions as appropriate.\n"
        "See documentation at \n  https://software.belle2.org/analysis/doc/Vertex.html"
    )

    B2WARNING(warning)

    _fitVertex(
        list_name,
        conf_level,
        decay_string,
        fitter,
        fit_type,
        constraint,
        daughtersUpdate,
        path,
    )


def _fitVertex(
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
    This is a private function, please use one of the aliases provided below.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit. 0 selects CL > 0
    @param decay_string select particles used for the vertex fit
    @param fitter       rave or kfitter
    @param fit_type     type of the kinematic fit (valid options are vertex/massvertex/mass)
    @param constraint   type of additional constraints (valid options are empty string/ipprofile/iptube/mother)
    @param daughtersUpdate make copy of the daughters and update them after the vertex fit
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
    silence_warning=True,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList.

    @param list_name       name of the input ParticleList
    @param conf_level      minimum value of the confidence level to accept the fit
    @param constraint      add aditional constraint to the fit (valid options are ipprofile or iptube)
    @param path            modules are added to this path
    @param decay_string    select particles used for the vertex fit
    @param silence_warning silence warning advertising TreeFitter use
    """

    message_a = (
        "Hello there. You seem to want to perform a vertex fit. Please consider using TreeFitter, even for single vertices.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ", updateAllDaughters=False)\n"
    )
    message_b = "To remove this warning, add silence_warning=True to the arguments of this function."

    if constraint is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at\n"
            "  https://software.belle2.org/analysis/doc/TreeFitter.html\n"
            "for special constraints.\n"
        )

    if not silence_warning:
        B2WARNING(message_a + message_if + message_b)

    _fitVertex(
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
    silence_warning=True,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList and update the Daughters.

    @param list_name      name of the input ParticleList
    @param conf_level      minimum value of the confidence level to accept the fit
    @param constraint      add aditional constraint to the fit (valid options are ipprofile or iptube)
    @param path            modules are added to this path
    @param silence_warning silence warning advertising TreeFitter use
    """

    message_a = (
        "Hello there. You seem to want to perform a vertex fit. Please consider using TreeFitter, even for single vertices.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ", updateAllDaughters=True)\n"
    )
    message_b = "To remove this warning, add silence_warning=True to the arguments of this function."

    if constraint is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at\n"
            "  https://software.belle2.org/analysis/doc/TreeFitter.html\n"
            "for special constraints.\n"
        )

    if not silence_warning:
        B2WARNING(message_a + message_if + message_b)

    _fitVertex(
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
    silence_warning=True,
):
    """
    Perform mass-constrained vertex fit using the kfitter for each Particle in the given ParticleList.

    @param list_name       name of the input ParticleList
    @param conf_level      minimum value of the confidence level to accept the fit
    @param path            modules are added to this path
    @param decay_string    select particles used for the vertex fit
    @param silence_warning silence warning advertising TreeFitter use
    """

    message_a = (
        "Hello there. You seem to want to perform a vertex fit. Please consider using TreeFitter, even for single vertices.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ",massConstraint=[\'" + list_name.split(':')[0] + "\'], updateAllDaughters=False)\n"
    )
    message_b = "To remove this warning, add silence_warning=True to the arguments of this function."

    if decay_string is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at\n"
            "  https://software.belle2.org/analysis/doc/TreeFitter.html\n"
            "for special constraints.\n"
        )

    if not silence_warning:
        B2WARNING(message_a + message_if + message_b)

    _fitVertex(
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
    silence_warning=True,
):
    """
    Perform mass-constrained vertex fit using the kfitter for each Particle in the given ParticleList and update the daughters.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    @param silence_warning silence warning advertising TreeFitter use
    """

    message_a = (
        "Hello there. You seem to want to perform a vertex fit. Please consider using TreeFitter, even for single vertices.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ",massConstraint=[\'" + list_name.split(':')[0] + "\'], updateAllDaughters=True)\n"
    )
    message_b = "To remove this warning, add silence_warning=True to the arguments of this function."

    if decay_string is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at\n"
            "  https://software.belle2.org/analysis/doc/TreeFitter.html\n"
            "for special constraints.\n"
        )

    if not silence_warning:
        B2WARNING(message_a + message_if + message_b)

    _fitVertex(
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
    silence_warning=True,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    @param silence_warning silence warning advertising TreeFitter use
    """

    message_a = (
        "Hello there. You seem to want to perform a vertex fit. Please consider using TreeFitter, even for single vertices.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ",massConstraint=[\'" + list_name.split(':')[0] + "\'], updateAllDaughters=False)\n"
    )
    message_b = "To remove this warning, add silence_warning=True to the arguments of this function."

    if decay_string is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at\n"
            "  https://software.belle2.org/analysis/doc/TreeFitter.html\n"
            "for special constraints.\n"
        )

    if not silence_warning:
        B2WARNING(message_a + message_if + message_b)

    _fitVertex(
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
    silence_warning=True,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList and update the daughters.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    @param silence_warning silence warning advertising TreeFitter use
    """

    message_a = (
        "Hello there. You seem to want to perform a vertex fit. Please consider using TreeFitter, even for single vertices.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ",massConstraint=[\'" + list_name.split(':')[0] + "\'], updateAllDaughters=True)\n"
    )
    message_b = "To remove this warning, add silence_warning=True to the arguments of this function."

    if decay_string is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at\n"
            "  https://software.belle2.org/analysis/doc/TreeFitter.html\n"
            "for special constraints.\n"
        )

    if not silence_warning:
        B2WARNING(message_a + message_if + message_b)

    _fitVertex(
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

    _fitVertex(
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

    _fitVertex(
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
    silence_warning=True,
):
    """
    Perform vertex fit using the RAVE for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param constraint   add aditional constraint to the fit (valid options are ipprofile or iptube)
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    @param silence_warning silence warning advertising TreeFitter use
    """

    message_a = (
        "Hello there. You seem to want to perform a vertex fit. Please consider using TreeFitter, even for single vertices.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ", updateAllDaughters=False)\n"
    )
    message_b = "To remove this warning, add silence_warning=True to the arguments of this function."

    if constraint is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at\n"
            "  https://software.belle2.org/analysis/doc/TreeFitter.html\n"
            "for special constraints.\n"
        )

    if not silence_warning:
        B2WARNING(message_a + message_if + message_b)

    _fitVertex(
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
    silence_warning=True,
):
    """
    Performs a vertex fit using RAVE for each Particle in the given ParticleList and updating fully the
    daughters used in the fit.
    The mother is only used in the fit if all daugthers (charged and neutral) are selected.
    For this, leave the decay_string empty.

    .. warning::
       For vertices with a single track the mother is not used.
       When a particle is fully updated its 4momentum, 3D vertex, covariance Matrix
       and p-value are updated.
       When the mother is not used in the fit, only its 3D vertex, covariance Matrix
       and p-value are updated.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param decay_string select particles used for the vertex fit (if given, only the selected particles are updated)
    @param constraint   add aditional constraint to the fit (valid options are ipprofile or iptube)
    @param path         modules are added to this path
    @param silence_warning silence warning advertising TreeFitter use
    """

    message_a = (
        "Hello there. You seem to want to perform a vertex fit. Please consider using TreeFitter, even for single vertices.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ", updateAllDaughters=True)\n"
    )
    message_b = "To remove this warning, add silence_warning=True to the arguments of this function."

    if constraint is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at\n"
            "  https://software.belle2.org/analysis/doc/TreeFitter.html\n"
            "for special constraints.\n"
        )

    if not silence_warning:
        B2WARNING(message_a + message_if + message_b)

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
    silence_warning=True,
):
    """
    Perform mass-constrained vertex fit using the RAVE for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    @param silence_warning silence warning advertising TreeFitter use
    """

    message_a = (
        "Hello there. You seem to want to perform a vertex fit. Please consider using TreeFitter, even for single vertices.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ",massConstraint=[\'" + list_name.split(':')[0] + "\'], updateAllDaughters=False)\n"
    )
    message_b = "To remove this warning, add silence_warning=True to the arguments of this function."

    if decay_string is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at\n"
            "  https://software.belle2.org/analysis/doc/TreeFitter.html\n"
            "for special constraints.\n"
        )

    if not silence_warning:
        B2WARNING(message_a + message_if + message_b)

    _fitVertex(
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
    silence_warning=True,
):
    """
    Perform mass-constrained vertex fit using the RAVE for each Particle in the given ParticleList and update the daughters.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    @param silence_warning silence warning advertising TreeFitter use
    """

    message_a = (
        "Hello there. You seem to want to perform a vertex fit. Please consider using TreeFitter, even for single vertices.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ",massConstraint=[\'" + list_name.split(':')[0] + "\'], updateAllDaughters=True)\n"
    )
    message_b = "To remove this warning, add silence_warning=True to the arguments of this function."

    if decay_string is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at\n"
            "  https://software.belle2.org/analysis/doc/TreeFitter.html\n"
            "for special constraints.\n"
        )

    if not silence_warning:
        B2WARNING(message_a + message_if + message_b)

    _fitVertex(
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
    silence_warning=True,
):
    """
    Perform mass fit using the RAVE for each Particle in the given ParticleList.
    7x7 error matrix of the mother particle must be defined

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    @param silence_warning silence warning advertising TreeFitter use
    """

    message_a = (
        "Hello there. You seem to want to perform a vertex fit. Please consider using TreeFitter, even for single vertices.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ",massConstraint=[\'" + list_name.split(':')[0] + "\'], updateAllDaughters=False)\n"
    )
    message_b = "To remove this warning, add silence_warning=True to the arguments of this function."

    if decay_string is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at\n"
            "  https://software.belle2.org/analysis/doc/TreeFitter.html\n"
            "for special constraints.\n"
        )

    if not silence_warning:
        B2WARNING(message_a + message_if + message_b)

    _fitVertex(
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

    :Example:
        An example of usage for the decay chain: math:`B^0\\to\\pi^+\\pi^-\\pi^0`  is the following:

    ::

      reconstructDecay('pi0:A -> gamma:pi0 gamma:pi0', '0.130 < InvM < 0.14')
      reconstructDecay('B0:treefit -> pi+:my pi-:my pi0:A ', '')
      vertexTree('B0:treefit', ipConstraint=True)

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit. 0 selects CL > 0
    @param massConstraint list of PDG ids or Names of the particles which are mass-constrained
      "Please do not mix PDG id and particle names in massConstraint list."
    @param ipConstraint constrain head production vertex to IP (x-y-z) constraint, default: False)
    @param customOriginConstraint use a costum origin vertex as the production vertex of your particle." + \
        "This is usefull when fitting D*/D without wanting to fit a B but constraining the process to be B-decay like + \
        "(think of semileptonic modes and stuff with a neutrino in the B decay). Default: False"
    @param customOriginVertex 3d vector of the vertex coordinates you want to use as custom origin."+\
        "Default numbers are taken for B-mesons
    @param customOriginCovariance 3x3 covariance matrix for the custom vertex (type: vector)." +\
        " Default numbers extracted from generator distribtuion width of B-mesons.
    @param updateAllDaughters if true the entire tree will be updated with the fitted values "+\
    "for momenta and vertex position. Otherwise only the momenta of the head of the tree will be updated, "+\
    "however for all daughters we also update the vertex position with the fit results as this would "+\
    "otherwise be set to {0, 0, 0} contact us if this causes any hardship/confusion.
    @param path         modules are added to this path
    """
    treeFitter = register_module("TreeFitter")
    treeFitter.set_name('TreeFitter_' + list_name)
    if massConstraint:
        if isinstance(massConstraint[0], str):
            treeFitter.param('massConstraintListParticlename', massConstraint)
        else:
            treeFitter.param('massConstraintList', massConstraint)
    treeFitter.param('particleList', list_name)
    treeFitter.param('confidenceLevel', conf_level)
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
    maskName='',
    path=analysis_main,
):
    """
    For each Particle in the given Breco ParticleList:
    perform the fit of tag side using the track list from the RestOfEvent dataobject
    save the MC Btag in case of signal MC

    @param list_name name of the input Breco ParticleList
    @param MCassociation: use standard MC association or the internal one
    @param confidenceLevel: minimum value of the ConfidenceLevel to accept the fit. 0 selects CL > 0
    @param useFitAlgorithm: choose the fit algorithm: boost, breco, standard, standard_pxd, singleTrack,
           singleTrack_pxd, noConstraint
    @param askMCInfo: True when requesting MC Information from the tracks performing the vertex fit
    @param reqPXDHits: minimum N PXD hits for a track
    @param maskName: get particles from a specified ROE mask
    @param path      modules are added to this path
    """

    tvfit = register_module('TagVertex')
    tvfit.set_name('TagVertex_' + list_name)
    tvfit.param('listName', list_name)
    tvfit.param('maskName', maskName)
    tvfit.param('confidenceLevel', confidenceLevel)
    tvfit.param('MCAssociation', MCassociation)
    tvfit.param('useFitAlgorithm', useFitAlgorithm)
    tvfit.param('askMCInformation', askMCInfo)
    tvfit.param('reqPXDHits', reqPXDHits)
    path.add_module(tvfit)


if __name__ == '__main__':
    from basf2.utils import pretty_print_module
    pretty_print_module(__name__, "vertex", {
        repr(analysis_main): "analysis_main",
    })


def fitPseudo(
    list_name,
    path=analysis_main,
):
    """
    Add a pseudo \"vertex fit\" which adds a covariance matrix from the combination of the four-vectors of the daughters.
    This is similar to BaBar's "Add4" function.
    It is commonly used for :math:`\\pi^0\\to\\gamma\\gamma` reconstruction where a vertex fit is not possible.

    Here is the basic usage:

    .. code-block::python
        from modularAnalysis import fitPseudo
        from stdPi0s import stdPi0s
        stdPi0s("loose", path=mypath)
        fitPseudo("pi0:loose", path=mypath)

    Parameters:
        list_name (str): the name of the list to add the covariance matrix to
        path (basf2.Path): modules are added to this path
    """

    pseudofit = register_module('PseudoVertexFitter')
    pseudofit.set_name('PseudoVertexFitter_' + list_name)
    pseudofit.param('listName', list_name)
    path.add_module(pseudofit)
