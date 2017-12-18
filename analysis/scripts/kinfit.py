#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import os
import sys
import inspect
from analysisPath import *


def fitKinematic4C(
    list_name,
    fitterEngine='NewFitterGSL',
    constraint='HardBeam',
    daughtersUpdate=True,
    addUnmeasuredPhoton=False,
    path=analysis_main,
):
    """
    Perform 4C momentum constraint kinematic fit for particles in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param fitterEngine       NewFitterGSL or OPALFitterGSL
    @param constraint       HardBeam or RecoilMass
    @param updateDaughters make copy of the daughters and update them after the vertex fit
    @param addUnmeasuredPhoton add one unmeasured photon (costs 3C)
    @param path         modules are added to this path
    """

    orca = register_module('ParticleKinematicFitter')
    orca.set_name('ParticleKinematicFitter_' + list_name)
    orca.param('debugFitter', False)
    orca.param('orcaTracer', 'None')
    orca.param('orcaFitterEngine', fitterEngine)
    orca.param('orcaConstraint', constraint)  # beam parameters automatically taken from database
    orca.param('listName', list_name)
    orca.param('updateDaughters', daughtersUpdate)
    orca.param('addUnmeasuredPhoton', addUnmeasuredPhoton)
    path.add_module(orca)


def fitKinematic1CUnmeasured(
    list_name,
    fitterEngine='NewFitterGSL',
    constraint='HardBeam',
    daughtersUpdate=True,
    addUnmeasuredPhoton=True,
    path=analysis_main,
):
    """
    Perform 1C momentum constraint kinematic fit with one unmeasured photon for particles in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param fitterEngine       NewFitterGSL or OPALFitterGSL
    @param constraint       HardBeam or RecoilMass
    @param updateDaughters make copy of the daughters and update them after the vertex fit
    @param addUnmeasuredPhoton add one unmeasured photon (costs 3C)
    @param path         modules are added to this path
    """

    orca = register_module('ParticleKinematicFitter')
    orca.set_name('ParticleKinematicFitter_' + list_name)
    orca.param('debugFitter', False)
    orca.param('orcaTracer', 'None')
    orca.param('orcaFitterEngine', fitterEngine)
    orca.param('orcaConstraint', constraint)  # beam parameters automatically taken from database
    orca.param('listName', list_name)
    orca.param('updateDaughters', daughtersUpdate)
    orca.param('addUnmeasuredPhoton', addUnmeasuredPhoton)
    path.add_module(orca)


def fitKinematic3C(
        list_name,
        fitterEngine='NewFitterGSL',
        constraint='HardBeam',
        daughtersUpdate=True,
        addUnmeasuredPhoton=False,
        add3CPhoton=True,
        path=analysis_main,
):
    """
    Perform 1C momentum constraint kinematic fit with one unmeasured photon for particles in the given        ParticleList.
    @param list_name    name of the input ParticleList
    @param fitterEngine       NewFitterGSL or OPALFitterGSL
    @param constraint       HardBeam or RecoilMass
    @param updateDaughters make copy of the daughters and update them after the vertex fit
    @param addUnmeasuredPhoton add one unmeasured photon (costs 3C)
    @param path         modules are added to this path
    """

    orca = register_module('ParticleKinematicFitter')
    orca.set_name('ParticleKinematicFitter_' + list_name)
    orca.param('debugFitter', True)
    orca.param('orcaTracer', 'None')
    orca.param('orcaFitterEngine', fitterEngine)
    orca.param('orcaConstraint', constraint)  # beam parameters automatically taken from database
    orca.param('listName', list_name)
    orca.param('updateDaughters', daughtersUpdate)
    orca.param('addUnmeasuredPhoton', addUnmeasuredPhoton)
    orca.param('add3CPhoton', add3CPhoton)
    path.add_module(orca)


def fitKinematic1CRecoilMass(
    list_name,
    recoilMass,
    fitterEngine='NewFitterGSL',
    constraint='RecoilMass',
    daughtersUpdate=True,
    addUnmeasuredPhoton=False,
    path=analysis_main,
):
    """
    Perform recoil mass kinematic fit for particles in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param fitterEngine       NewFitterGSL or OPALFitterGSL
    @param constraint       HardBeam or RecoilMass
    @param recoilMass       RecoilMass (GeV)
    @param updateDaughters make copy of the daughters and update them after the vertex fit
    @param addUnmeasuredPhoton add one unmeasured photon (costs 3C)
    @param path         modules are added to this path
    """

    orca = register_module('ParticleKinematicFitter')
    orca.set_name('ParticleKinematicFitter_' + list_name)
    orca.param('debugFitter', False)
    orca.param('orcaTracer', 'None')
    orca.param('orcaFitterEngine', fitterEngine)
    orca.param('orcaConstraint', constraint)
    orca.param('recoilMass', recoilMass)
    orca.param('listName', list_name)
    orca.param('updateDaughters', daughtersUpdate)
    orca.param('addUnmeasuredPhoton', addUnmeasuredPhoton)
    path.add_module(orca)


def fitKinematic1CMass(
    list_name,
    invMass,
    fitterEngine='NewFitterGSL',
    constraint='Mass',
    daughtersUpdate=True,
    addUnmeasuredPhoton=False,
    path=analysis_main,
):
    """
    Perform recoil mass kinematic fit for particles in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param fitterEngine       NewFitterGSL or OPALFitterGSL
    @param constraint       HardBeam or RecoilMass or Mass
    @param invMass       Inviriant  Mass (GeV)
    @param updateDaughters make copy of the daughters and update them after the vertex fit
    @param addUnmeasuredPhoton add one unmeasured photon (costs 3C)
    @param path         modules are added to this path
    """

    orca = register_module('ParticleKinematicFitter')
    orca.set_name('ParticleKinematicFitter_' + list_name)
    orca.param('debugFitter', False)
    orca.param('orcaTracer', 'None')
    orca.param('orcaFitterEngine', fitterEngine)
    orca.param('orcaConstraint', constraint)
    orca.param('invMass', invMass)
    orca.param('listName', list_name)
    orca.param('updateDaughters', daughtersUpdate)
    orca.param('addUnmeasuredPhoton', addUnmeasuredPhoton)
    path.add_module(orca)


if __name__ == '__main__':
    desc_list = []
    for function_name in sorted(list_functions(sys.modules[__name__])):
        function = globals()[function_name]
        signature = inspect.formatargspec(*inspect.getargspec(function))
        signature = signature.replace(repr(analysis_main), 'analysis_main')
        desc_list.append((function.__name__, signature + '\n' + function.__doc__))
    pretty_print_description_list(desc_list)
