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
    smearing=0,
    path=None,
):
    """
    Perform the specified fit for each Particle in the given ParticleList.

    Info:
        Direct use of `fitVertex` is not recommended unless you know what you are doing.
        If you're unsure, you probably want to use `treeFit` or `KFit`.

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
        smearing (float) :      IP tube width is smeared by this value (cm). meaningful only with 'kfitter/vertex/iptube' option.
        path (basf2.Path):      modules are added to this path
    """

    warning = (
        "Direct use of fitVertex is not recommended unless you know what you are doing.\n"
        "Please use treeFit or KFit.\n"
        "See documentation at https://software.belle2.org"
    )

    B2WARNING(warning)

    _fitVertex(list_name, conf_level, decay_string, fitter, fit_type, constraint, daughtersUpdate, smearing, path)


def _fitVertex(
    list_name,
    conf_level,
    decay_string='',
    fitter='kfitter',
    fit_type='vertex',
    constraint='',
    daughtersUpdate=False,
    smearing=0,
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
        smearing (float) :      IP tube width is smeared by this value (cm). meaningful only with 'kfitter/vertex/iptube' option.
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
    pvfit.param('smearing', smearing)
    path.add_module(pvfit)


def KFit(list_name,
         conf_level,
         fit_type='vertex',
         constraint='',
         daughtersUpdate=False,
         decay_string='',
         smearing=0,
         path=None):
    """
    Perform KFit for each Particle in the given ParticleList.

    Parameters:
        list_name (str):        name of the input ParticleList
        conf_level (float):     minimum value of the confidence level to accept the fit
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects particle candidates with a failed fit.
        fit_type (str):         type of the kinematic fit. Valid options are

          * ``mass`` for a mass-constrained fit
          * ``vertex`` for a vertex fit
          * ``massvertex`` for a vertex fit with a mass constraint on the mother particle
          * ``fourC`` for a vertex fit in which the mother particle's four-momentum is constrained to the beam four-momentum

        constraint (str):       add an additional constraint to the fit (valid options are ipprofile or iptube)
        daughtersUpdate( bool): make copy of the daughters and update them after the KFit
        decay_string (str):     select particles used for the KFit
        smearing (float) :      IP tube width is smeared by this value (cm). meaningful only with 'iptube' constraint.
        path (basf2.Path):      modules are added to this path
    """

    _fitVertex(list_name, conf_level, decay_string, 'kfitter', fit_type, constraint, daughtersUpdate, smearing, path)


def vertexKFit(
    list_name,
    conf_level,
    decay_string='',
    constraint='',
    smearing=0,
    path=None,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList.

    Warning:
        This function is no longer recommended and will be removed in release-05.
        It is replaced by the function `KFit` which provides the same
        functionality if you set the parameter ``fit_type`` to ``vertex``.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        constraint (str):   add aditional constraint to the fit (valid options are ipprofile or iptube)
        smearing (float) :  IP tube width is smeared by this value (cm). meaningful only with 'iptube' constraint.
        path (basf2.Path):  modules are added to this path
    """

    B2WARNING("This function is no longer recommended and will be removed in release-05.\n"
              "It is replaced by the function KFit which provides the same functionality"
              "if you set the parameter fit_type to vertex.")

    _fitVertex(list_name, conf_level, decay_string, 'kfitter', 'vertex', constraint, False, smearing, path)


def vertexKFitDaughtersUpdate(
    list_name,
    conf_level,
    constraint='',
    smearing=0,
    path=None,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList and update the Daughters.

    Warning:
        This function is no longer recommended and will be removed in release-05.
        It is replaced by the function `KFit` which provides the same functionality
        if you set the parameter ``fit_type`` to ``vertex`` and the parameter ``daughtersUpdate`` to ``True``.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        constraint (str):   add aditional constraint to the fit (valid options are ipprofile or iptube)
        smearing (float) :  IP tube width is smeared by this value (cm). meaningful only with 'iptube' constraint.
        path (basf2.Path):  modules are added to this path
    """

    B2WARNING("This function is no longer recommended and will be removed in release-05.\n"
              "It is replaced by the function KFit which provides the same functionality"
              "if you set the parameter fit_type to vertex and the parameter daughtersUpdate to True.")

    _fitVertex(list_name, conf_level, '', 'kfitter', 'vertex', constraint, True, smearing, path)


def massVertexKFit(
    list_name,
    conf_level,
    decay_string='',
    path=None,
):
    """
    Perform mass-constrained vertex fit using the kfitter for each Particle in the given ParticleList.

    Warning:
        This function is no longer recommended and will be removed in release-05.
        It is replaced by the function `KFit` which provides the same functionality
        if you set the parameter ``fit_type`` to ``massvertex``.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        path (basf2.Path):  modules are added to this path
    """

    B2WARNING("This function is no longer recommended and will be removed in release-05.\n"
              "It is replaced by the function KFit which provides the same functionality"
              "if you set the parameter fit_type to massvertex.")

    _fitVertex(list_name, conf_level, decay_string, 'kfitter', 'massvertex', '', False, 0, path)


def massVertexKFitDaughtersUpdate(
    list_name,
    conf_level,
    decay_string='',
    path=None,
):
    """
    Perform mass-constrained vertex fit using the kfitter for each Particle in the given ParticleList and update the daughters.

    Warning:
        This function is no longer recommended and will be removed in release-05.
        It is replaced by the function `KFit` which provides the same functionality
        if you set the parameter ``fit_type`` to ``massvertex`` and the parameter ``daughtersUpdate`` to ``True``.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        path (basf2.Path):  modules are added to this path
    """

    B2WARNING("This function is no longer recommended and will be removed in release-05.\n"
              "It is replaced by the function KFit which provides the same functionality"
              "if you set the parameter fit_type to massvertex and the parameter daughtersUpdate to True.")

    _fitVertex(list_name, conf_level, decay_string, 'kfitter', 'massvertex', '', True, 0, path)


def massKFit(
    list_name,
    conf_level,
    decay_string='',
    path=None,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList.

    Warning:
        This function is no longer recommended and will be removed in release-05.
        It is replaced by the function `KFit` which provides the same functionality
        if you set the parameter ``fit_type`` to ``mass``.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        path (basf2.Path):  modules are added to this path
    """

    B2WARNING("This function is no longer recommended and will be removed in release-05.\n"
              "It is replaced by the function KFit which provides the same functionality"
              "if you set the parameter fit_type to mass.")

    _fitVertex(list_name, conf_level, decay_string, 'kfitter', 'mass', '', False, 0, path)


def massKFitDaughtersUpdate(
    list_name,
    conf_level,
    decay_string='',
    path=None,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList and update the daughters.

    Warning:
        This function is no longer recommended and will be removed in release-05.
        It is replaced by the function `KFit` which provides the same functionality
        if you set the parameter ``fit_type`` to ``mass`` and the parameter ``daughtersUpdate`` to ``True``.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        path (basf2.Path):  modules are added to this path
    """

    B2WARNING("This function is no longer recommended and will be removed in release-05.\n"
              "It is replaced by the function KFit which provides the same functionality"
              "if you set the parameter fit_type to mass and the parameter daughtersUpdate to True.")

    _fitVertex(list_name, conf_level, decay_string, 'kfitter', 'mass', '', True, 0, path)


def fourCKFit(
    list_name,
    conf_level,
    decay_string='',
    path=None,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList.

    Warning:
        This function is no longer recommended and will be removed in release-05.
        It is replaced by the function `KFit` which provides the same functionality
        if you set the parameter ``fit_type`` to ``fourC``.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        path (basf2.Path):  modules are added to this path
    """

    B2WARNING("This function is no longer recommended and will be removed in release-05.\n"
              "It is replaced by the function KFit which provides the same functionality"
              "if you set the parameter fit_type to fourC.")

    _fitVertex(list_name, conf_level, decay_string, 'kfitter', 'fourC', '', False, 0, path)


def fourCKFitDaughtersUpdate(
    list_name,
    conf_level,
    decay_string='',
    path=None,
):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList and update the daughters.

    Warning:
        This function is no longer recommended and will be removed in release-05.
        It is replaced by the function `KFit` which provides the same functionality
        if you set the parameter ``fit_type`` to ``fourC`` and the parameter ``daughtersUpdate`` to ``True``.

    Parameters:
        list_name (str):    name of the input ParticleList
        conf_level (float): minimum value of the confidence level to accept the fit.
            Setting this parameter to -1 selects all particle candidates.
            The value of 0 rejects the particle candidates with failed fit.
        decay_string (str): select particles used for the vertex fit
        path (basf2.Path):  modules are added to this path
    """

    B2WARNING("This function is no longer recommended and will be removed in release-05.\n"
              "It is replaced by the function KFit which provides the same functionality"
              "if you set the parameter fit_type to fourC and the parameter daughtersUpdate to True.")

    _fitVertex(list_name, conf_level, decay_string, 'kfitter', 'fourC', '', True, 0, path)


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
        Instead, we recommend :doc:`TreeFitter` (`vertex.treeFit`) or `vertex.KFit`.

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
        "Instead, we recommend TreeFitter (treeFit) or KFit.\n"
        "Try: \n  treeFit(\'" + list_name + "\'," + str(conf_level) +
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

    _fitVertex(list_name, conf_level, decay_string, 'rave', 'vertex', constraint, False, 0, path)


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
        Instead, we recommend :doc:`TreeFitter` (`vertex.treeFit`) or `vertex.KFit`.

    The mother is only used in the fit if all daugthers (charged and neutral) are selected.
    For this, leave the ``decay_string`` empty.

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
        "Instead, we recommend TreeFitter (treeFit) or KFit.\n"
        "Try: \n  treeFit(\'" + list_name + "\'," + str(conf_level) +
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
        Instead, we recommend :doc:`TreeFitter` (`vertex.treeFit`) or `vertex.KFit`.

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
        "Instead, we recommend TreeFitter (treeFit) or KFit.\n"
        "Try: \n  treeFit(\'" + list_name + "\'," + str(conf_level) +
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

    _fitVertex(list_name, conf_level, decay_string, 'rave', 'massvertex', '', False, 0, path)


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
        Instead, we recommend :doc:`TreeFitter` (`vertex.treeFit`) or `vertex.KFit`.

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
        "Instead, we recommend TreeFitter (treeFit) or KFit.\n"
        "Try: \n  treeFit(\'" + list_name + "\'," + str(conf_level) +
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

    _fitVertex(list_name, conf_level, decay_string, 'rave', 'massvertex', '', True, 0, path)


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
        Instead, we recommend :doc:`TreeFitter` (`vertex.treeFit`) or `vertex.KFit`.

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
        "Instead, we recommend TreeFitter (treeFit) or KFit.\n"
        "Try: \n  treeFit(\'" + list_name + "\'," + str(conf_level) +
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

    _fitVertex(list_name, conf_level, decay_string, 'rave', 'mass', '', False, 0, path)


def treeFit(
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
    Perform a :doc:`TreeFitter` fit for each Particle in the given ParticleList.

    :Example:
        An example of usage for the decay chain :math:`B^0\\to\\pi^+\\pi^-\\pi^0`  is the following:

    ::

      reconstructDecay('pi0:A -> gamma:pi0 gamma:pi0', '0.130 < InvM < 0.14', path=mypath)
      reconstructDecay('B0:treefit -> pi+:my pi-:my pi0:A ', '', path=mypath)
      treeFit('B0:treefit', ipConstraint=True, path=mypath)

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
    trackFindingType="standard_PXD",
    constraintType="IP",
    askMCInfo=False,
    reqPXDHits=0,
    maskName='',
    fitAlgorithm='Rave',
    useTruthInFit=False,
    path=None,
):
    """
    For each Particle in the given Breco ParticleList:
    perform the fit of tag side using the track list from the RestOfEvent dataobject
    save the MC Btag in case of signal MC

    Parameters:

        list_name (str): name of the input Breco ParticleList
        MCassociation (str): use standard MC association or the internal one
        confidenceLevel (float): minimum value of the ConfidenceLevel to accept the fit. 0 selects CL > 0
        constraintType (str): choose the constraint used in the fit. Can be set to

          * noConstraint;
          * IP: **default**, tag B constrained to be on the IP;
          * tube: tube along the tag B line of flight, only for fully reconstructed signal B;
          * boost: long tube along the boost direction;
          * (breco): deprecated, but similar to tube;

        trackFindingType (str): choose how to look for tag tracks. Can be set to

          * standard: all tracks except from Kshorts;
          * standard_PXD: **default**, same as above but consider only tracks with at least 1 PXD hit;
          * singleTrack: only choose the best track, DOES NOT WORK with no constraint;
          * singleTrack_PXD: same as above but consider only tracks with at least 1 PXD hit;

        fitAlgorithm (str):     Fitter used for the tag vertex fit: Rave (default) or KFit
        askMCInfo (bool): True when requesting MC Information from the tracks performing the vertex fit
        reqPXDHits (int): minimum N PXD hits for a track (default is 0)
        maskName (str): get particles from a specified ROE mask
        useTruthInFit (bool): True when the tag vertex fit is performed with the true momentum
        and position of the tracks (default is false). The variable "TagVFitTruthStatus" is set to 1 if the
        truth-matching succeeds and 2 otherwise.
        path (basf2.Path): modules are added to this path

    Warning:
        Note that the useFitAlgorithm (str) parameter is deprecated and replaced by constraintType (str)
        and trackFindingType (str)
    """

    tvfit = register_module('TagVertex')
    tvfit.set_name('TagVertex_' + list_name)
    tvfit.param('listName', list_name)
    tvfit.param('maskName', maskName)
    tvfit.param('confidenceLevel', confidenceLevel)
    tvfit.param('MCAssociation', MCassociation)
    tvfit.param('trackFindingType', trackFindingType)
    tvfit.param('constraintType', constraintType)
    tvfit.param('askMCInformation', askMCInfo)
    tvfit.param('reqPXDHits', reqPXDHits)
    tvfit.param('fitAlgorithm', fitAlgorithm)
    tvfit.param('useTruthInFit', useTruthInFit)
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
