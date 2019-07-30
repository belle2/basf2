#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import register_module
from basf2 import B2WARNING


def fitVertex(
    list_name,
    conf_level,
    decay_string='',
    fitter='kfitter',
    fit_type='vertex',
    constraint='',
    daughtersUpdate=False,
    path=None,
):
    """
    Perform the specified fit for each Particle in the given ParticleList.

    Info:
        Direct use of `fitVertex` is not recommended unless you know what you are doing.
        If you're unsure, you probably want to use `vertexTree`, `vertexKFit`
        or another convenience function where the fitter is configured for you.

    Parameters:
        list_name (str):        name of the input ParticleList
        conf_level (float):     minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str):     select particles used for the vertex fit
        fitter (str):           rave or kfitter
        fit_type (str):         type of the kinematic fit (valid options are vertex/massvertex/mass/fourC)
        constraint (str):       add aditional constraint to the fit (valid options are empty string/ipprofile/iptube/mother)
        daughtersUpdate (bool): make copy of the daughters and update them after the vertex fit
        path (basf2.Path):      modules are added to this path
    """

    warning = (
        "Direct use of fitVertex is not recommended unless you know what you are doing.\n"
        "Please use vertexTree, vertexKFit, or any of the other convenience functions as appropriate.\n"
        "See documentation at  https://software.belle2.org"
    )

    B2WARNING(warning)

    _fitVertex(list_name, conf_level, decay_string, fitter, fit_type, constraint, daughtersUpdate, path)


def _fitVertex(
    list_name,
    conf_level,
    decay_string='',
    fitter='kfitter',
    fit_type='vertex',
    constraint='',
    daughtersUpdate=False,
    path=None,
):
    """
    An internal function, performs the specified fit for each Particle in the given ParticleList.

    Warning:
        This is a private function, please use one of the aliases provided.

    Parameters:
        list_name (str):        name of the input ParticleList
        conf_level (float):     minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str):     select particles used for the vertex fit
        fitter (str):           rave or kfitter
        fit_type (str):         type of the kinematic fit (valid options are vertex/massvertex/mass/fourC)
        constraint (str):       add aditional constraint to the fit (valid options are empty string/ipprofile/iptube/mother)
        daughtersUpdate (bool): make copy of the daughters and update them after the vertex fit
        path (basf2.Path):      modules are added to this path
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
    path=None,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        constraint (str):   add aditional constraint to the fit (valid options are ipprofile or iptube)
        path (basf2.Path):  modules are added to this path
    """

    _fitVertex(list_name, conf_level, decay_string, 'kfitter', 'vertex', constraint, False, path)


def vertexKFitDaughtersUpdate(
    list_name,
    conf_level,
    constraint='',
    path=None,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList and update the Daughters.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        constraint (str):   add aditional constraint to the fit (valid options are ipprofile or iptube)
        path (basf2.Path):  modules are added to this path
    """

    _fitVertex(list_name, conf_level, '', 'kfitter', 'vertex', constraint, True, path)


def massVertexKFit(
    list_name,
    conf_level,
    decay_string='',
    path=None,
):
    """
    Perform mass-constrained vertex fit using the kfitter for each Particle in the given ParticleList.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        path (basf2.Path):  modules are added to this path
    """

    _fitVertex(list_name, conf_level, decay_string, 'kfitter', 'massvertex', '', False, path)


def massVertexKFitDaughtersUpdate(
    list_name,
    conf_level,
    decay_string='',
    path=None,
):
    """
    Perform mass-constrained vertex fit using the kfitter for each Particle in the given ParticleList and update the daughters.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        path (basf2.Path):  modules are added to this path
    """

    _fitVertex(list_name, conf_level, decay_string, 'kfitter', 'massvertex', '', True, path)


def massKFit(
    list_name,
    conf_level,
    decay_string='',
    path=None,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        path (basf2.Path):  modules are added to this path
    """

    _fitVertex(list_name, conf_level, decay_string, 'kfitter', 'mass', '', False, path)


def massKFitDaughtersUpdate(
    list_name,
    conf_level,
    decay_string='',
    path=None,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList and update the daughters.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        path (basf2.Path):  modules are added to this path
    """

    _fitVertex(list_name, conf_level, decay_string, 'kfitter', 'mass', '', True, path)


def fourCKFit(
    list_name,
    conf_level,
    decay_string='',
    path=None,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        path (basf2.Path):  modules are added to this path
    """

    _fitVertex(list_name, conf_level, decay_string, 'kfitter', 'fourC', '', False, path)


def fourCKFitDaughtersUpdate(
    list_name,
    conf_level,
    decay_string='',
    path=None,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList and update the daughters.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        path (basf2.Path):  modules are added to this path
    """

    _fitVertex(list_name, conf_level, decay_string, 'kfitter', 'fourC', '', True, path)


def vertexRave(
    list_name,
    conf_level,
    decay_string='',
    constraint='',
    path=None,
    silence_warning=False,
):
    """
    Performs a vertex fit using `RAVE <https://github.com/rave-package>`_
    for each Particle in the given ParticleList.

    Warning:
        `RAVE <https://github.com/rave-package>`_ is deprecated since it is not maintained.
        Whilst we will not remove RAVE, it is not recommended for analysis use, other than benchmarking or legacy studies.
        Instead, we recommend `TreeFitter` (`vertex.vertexTree`) or `vertex.vertexKFit`.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        constraint (str):   add aditional constraint to the fit
            (valid options are ipprofile or iptube).
        path (basf2.Path):  modules are added to this path
        silence_warning (bool): silence the warning advertising TreeFitter use
    """

    # verbose deprecation message
    message_a = (
        "RAVE is deprecated since it is not maintained.\n"
        "Whilst we will not remove RAVE, it is not recommended for analysis use, other than benchmarking or legacy studies.\n"
        "Instead, we recommend TreeFitter (vertexTree) or vertexKFit.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ", updateAllDaughters=False, path=mypath)\n"
    )
    message_b = "To silence this warning, add silence_warning=True when you call this function."

    # if the user wants a constraint, they should check the doc, or send a ticket if it's not implemented
    if constraint is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at  https://software.belle2.org \n"
            "(search for TreeFitter) for special constraints.\n"
        )

    if not silence_warning:
        B2WARNING(message_a + message_if + message_b)

    _fitVertex(list_name, conf_level, decay_string, 'rave', 'vertex', constraint, False, path)


def vertexRaveDaughtersUpdate(
    list_name,
    conf_level,
    decay_string='',
    constraint='',
    path=None,
    silence_warning=False,
):
    """
    Performs a vertex fit using RAVE for each Particle in the given ParticleList and updating fully the
    daughters used in the fit.

    Warning:
        `RAVE <https://github.com/rave-package>`_ is deprecated since it is not maintained.
        Whilst we will not remove RAVE, it is not recommended for analysis use, other than benchmarking or legacy studies.
        Instead, we recommend `TreeFitter` (`vertex.vertexTree`) or `vertex.vertexKFit`.

    The mother is only used in the fit if all daugthers (charged and neutral) are selected.
    For this, leave the `decay_string` empty.

    Information:
       For vertices with a single track the mother is not used.
       When a particle is fully updated its 4momentum, 3D vertex, covariance Matrix
       and p-value are updated.
       When the mother is not used in the fit, only its 3D vertex, covariance Matrix
       and p-value are updated.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        constraint (str):   add aditional constraint to the fit
            (valid options are ipprofile or iptube).
        path (basf2.Path):  modules are added to this path
        silence_warning (bool): silence the warning advertising TreeFitter use
    """

    # verbose deprecation message
    message_a = (
        "RAVE is deprecated since it is not maintained.\n"
        "Whilst we will not remove RAVE, it is not recommended for analysis use, other than benchmarking or legacy studies.\n"
        "Instead, we recommend TreeFitter (vertexTree) or vertexKFit.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ", updateAllDaughters=True, path=mypath)\n"
    )
    message_b = "To silence this warning, add silence_warning=True when you call this function."

    # if the user wants a constraint, they should check the doc, or send a ticket if it's not implemented
    if constraint is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at  https://software.belle2.org \n"
            "(search for TreeFitter) for special constraints.\n"
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
    path=None,
    silence_warning=False,
):
    """
    Perform mass-constrained vertex fit using the RAVE for each Particle in the given ParticleList.

    Warning:
        `RAVE <https://github.com/rave-package>`_ is deprecated since it is not maintained.
        Whilst we will not remove RAVE, it is not recommended for analysis use, other than benchmarking or legacy studies.
        Instead, we recommend `TreeFitter` (`vertex.vertexTree`) or `vertex.vertexKFit`.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        path (basf2.Path):  modules are added to this path
        silence_warning (bool): silence the warning advertising TreeFitter use
    """

    # verbose deprecation message
    message_a = (
        "RAVE is deprecated since it is not maintained.\n"
        "Whilst we will not remove RAVE, it is not recommended for analysis use, other than benchmarking or legacy studies.\n"
        "Instead, we recommend TreeFitter (vertexTree) or vertexKFit.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ",massConstraint=[\'" + list_name.split(':')[0] + "\'], updateAllDaughters=False, path=mypath)\n"
    )
    message_b = "To silence this warning, add silence_warning=True when you call this function."

    # if the user wants a constraint, they should check the doc, or send a ticket if it's not implemented
    if decay_string is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at  https://software.belle2.org \n"
            "(search for TreeFitter) for special constraints.\n"
        )

    if not silence_warning:
        B2WARNING(message_a + message_if + message_b)

    _fitVertex(list_name, conf_level, decay_string, 'rave', 'massvertex', '', False, path)


def massVertexRaveDaughtersUpdate(
    list_name,
    conf_level,
    decay_string='',
    path=None,
    silence_warning=False,
):
    """
    Perform mass-constrained vertex fit using the RAVE for each Particle in the given ParticleList and update the daughters.

    Warning:
        `RAVE <https://github.com/rave-package>`_ is deprecated since it is not maintained.
        Whilst we will not remove RAVE, it is not recommended for analysis use, other than benchmarking or legacy studies.
        Instead, we recommend `TreeFitter` (`vertex.vertexTree`) or `vertex.vertexKFit`.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        path (basf2.Path):  modules are added to this path
        silence_warning (bool): silence the warning advertising TreeFitter use
    """

    # verbose deprecation message
    message_a = (
        "RAVE is deprecated since it is not maintained.\n"
        "Whilst we will not remove RAVE, it is not recommended for analysis use, other than benchmarking or legacy studies.\n"
        "Instead, we recommend TreeFitter (vertexTree) or vertexKFit.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ",massConstraint=[\'" + list_name.split(':')[0] + "\'], updateAllDaughters=True, path=mypath)\n"
    )
    message_b = "To silence this warning, add silence_warning=True when you call this function."

    # if the user wants a constraint, they should check the doc, or send a ticket if it's not implemented
    if decay_string is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at  https://software.belle2.org \n"
            "(search for TreeFitter) for special constraints.\n"
        )

    if not silence_warning:
        B2WARNING(message_a + message_if + message_b)

    _fitVertex(list_name, conf_level, decay_string, 'rave', 'massvertex', '', True, path)


def massRave(
    list_name,
    conf_level,
    decay_string='',
    path=None,
    silence_warning=False,
):
    """
    Perform mass fit using the RAVE for each Particle in the given ParticleList.
    7x7 error matrix of the mother particle must be defined

    Warning:
        `RAVE <https://github.com/rave-package>`_ is deprecated since it is not maintained.
        Whilst we will not remove RAVE, it is not recommended for analysis use, other than benchmarking or legacy studies.
        Instead, we recommend `TreeFitter` (`vertex.vertexTree`) or `vertex.vertexKFit`.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        path (basf2.Path):  modules are added to this path
        silence_warning (bool): silence the warning advertising TreeFitter use
    """

    # verbose deprecation message
    message_a = (
        "RAVE is deprecated since it is not maintained.\n"
        "Whilst we will not remove RAVE, it is not recommended for analysis use, other than benchmarking or legacy studies.\n"
        "Instead, we recommend TreeFitter (vertexTree) or vertexKFit.\n"
        "Try: \n  vertexTree(\'" + list_name + "\'," + str(conf_level) +
        ",massConstraint=[\'" + list_name.split(':')[0] + "\'], updateAllDaughters=False, path=mypath)\n"
    )
    message_b = "To silence this warning, add silence_warning=True when you call this function."

    # if the user wants a constraint, they should check the doc, or send a ticket if it's not implemented
    if decay_string is '':
        message_if = ""
    else:
        message_if = (
            "Please consult the documentation at  https://software.belle2.org \n"
            "(search for TreeFitter) for special constraints.\n"
        )

    if not silence_warning:
        B2WARNING(message_a + message_if + message_b)

    _fitVertex(list_name, conf_level, decay_string, 'rave', 'mass', '', False, path)


def vertexTree(
    list_name,
    conf_level=0.001,
    massConstraint=[],
    ipConstraint=False,
    updateAllDaughters=False,
    customOriginConstraint=False,
    customOriginVertex=[0.001, 0, 0.0116],
    customOriginCovariance=[0.0048, 0, 0, 0, 0.003567, 0, 0, 0, 0.0400],
    path=None,
):
    """
    Perform a `TreeFitter` fit for each Particle in the given ParticleList.

    :Example:
        An example of usage for the decay chain :math:`B^0\\to\\pi^+\\pi^-\\pi^0`  is the following:

    ::

      reconstructDecay('pi0:A -> gamma:pi0 gamma:pi0', '0.130 < InvM < 0.14', path=mypath)
      reconstructDecay('B0:treefit -> pi+:my pi-:my pi0:A ', '', path=mypath)
      vertexTree('B0:treefit', ipConstraint=True, path=mypath)

    Parameters:
        list_name (str):     name of the input ParticleList
        conf_level (float):  minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        massConstraint (list(int) or list(str)): list of PDG ids or Names of the particles which are mass-constrained
            Please do not mix PDG id and particle names in massConstraint list.
        ipConstraint (bool): constrain head production vertex to IP (x-y-z) constraint
        customOriginConstraint (bool): use a costum origin vertex as the production vertex of your particle.
            This is usefull when fitting D*/D without wanting to fit a B but constraining the process to be B-decay-like.
            (think of semileptonic modes and stuff with a neutrino in the B decay).
        customOriginVertex (list(float)): 3d vector of the vertex coordinates you want to use as custom origin.
            Default numbers are taken for B-mesons
        customOriginCovariance (list(float)): 3x3 covariance matrix for the custom vertex (type: vector).
            Default numbers extracted from generator distribtuion width of B-mesons.
        updateAllDaughters (bool): if true the entire tree will be updated with the fitted values
            for momenta and vertex position. Otherwise only the momenta of the head of the tree will be updated,
            however for all daughters we also update the vertex position with the fit results as this would
            otherwise be set to {0, 0, 0} contact us if this causes any hardship/confusion.
        path (basf2.Path): modules are added to this path
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
    path=None,
):
    """
    For each Particle in the given Breco ParticleList:
    perform the fit of tag side using the track list from the RestOfEvent dataobject
    save the MC Btag in case of signal MC

    Parameters:
        list_name (str):         name of the input Breco ParticleList
        MCassociation (str):     use standard MC association or the internal one
        confidenceLevel (float): minimum value of the ConfidenceLevel to accept the fit. 0 selects CL > 0
        useFitAlgorithm (str):   choose the fit algorithm: boost, breco, standard, standard_pxd, singleTrack,
            singleTrack_pxd, noConstraint
        askMCInfo (bool):       True when requesting MC Information from the tracks performing the vertex fit
        reqPXDHits (int):       minimum N PXD hits for a track
        maskName (str):         get particles from a specified ROE mask
        path (basf2.Path):      modules are added to this path
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


def fitPseudo(
    list_name,
    path,
):
    """
    Add a pseudo \"vertex fit\" which adds a covariance matrix from the combination of the four-vectors of the daughters.
    This is similar to BaBar's "Add4" function.
    It is commonly used for :math:`\\pi^0\\to\\gamma\\gamma` reconstruction where a vertex fit is not possible.

    Here is the basic usage:

    ::

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


if __name__ == '__main__':
    from basf2.utils import pretty_print_module
    pretty_print_module(__name__, "vertex")
