#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import register_module


def fitKinematic4C(
    list_name,
    fitterEngine='NewFitterGSL',
    constraint='HardBeam',
    daughtersUpdate=True,
    addUnmeasuredPhoton=False,
    path=None,
):
    """
    Perform a 4C momentum constraint kinematic fit for particles in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param fitterEngine which fitter engine to use? 'NewFitterGSL' or 'OPALFitterGSL'
    @param constraint       HardBeam or RecoilMass
    @param daughtersUpdate make copy of the daughters and update them after the vertex fit
    @param addUnmeasuredPhoton add one unmeasured photon (uses up three constraints)
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


def UnmeasuredfitKinematic1C(
    list_name,
    fitterEngine='NewFitterGSL',
    constraint='HardBeam',
    daughtersUpdate=True,
    path=None,
):
    """
    Perform 1C momentum constraint kinematic fit with one unmeasured photon for particles in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param fitterEngine which fitter engine to use? 'NewFitterGSL' or 'OPALFitterGSL'
    @param constraint       HardBeam or RecoilMass
    @param daughtersUpdate make copy of the daughters and update them after the vertex fit
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
    orca.param('addUnmeasuredPhoton', True)
    path.add_module(orca)


def fitKinematic3C(
        list_name,
        fitterEngine='NewFitterGSL',
        constraint='HardBeam',
        daughtersUpdate=True,
        addUnmeasuredPhoton=False,
        add3CPhoton=True,
        path=None,
):
    """
    Perform 3C momentum constraint kinematic fit with one photon with unmeasured energy for particles
    in the given ParticleList, the first daughter should be the energy unmeasured Photon.

    @param list_name    name of the input ParticleList
    @param fitterEngine which fitter engine to use? 'NewFitterGSL' or 'OPALFitterGSL'
    @param constraint       HardBeam or RecoilMass
    @param daughtersUpdate make copy of the daughters and update them after the vertex fit
    @param addUnmeasuredPhoton add one unmeasured photon (uses up three constraints)
    @param add3CPhoton add one photon with unmeasured energy (uses up a constraint)
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
    orca.param('add3CPhoton', add3CPhoton)
    path.add_module(orca)


def MassfitKinematic1CRecoil(
    list_name,
    recoilMass,
    fitterEngine='NewFitterGSL',
    constraint='RecoilMass',
    daughtersUpdate=True,
    path=None,
):
    """
    Perform recoil mass kinematic fit for particles in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param fitterEngine which fitter engine to use? 'NewFitterGSL' or 'OPALFitterGSL'
    @param constraint       HardBeam or RecoilMass
    @param recoilMass       RecoilMass (GeV)
    @param daughtersUpdate make copy of the daughters and update them after the vertex fit
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
    orca.param('addUnmeasuredPhoton', False)
    path.add_module(orca)


def MassfitKinematic1C(
    list_name,
    invMass,
    fitterEngine='NewFitterGSL',
    constraint='Mass',
    daughtersUpdate=True,
    path=None,
):
    """
    Perform recoil mass kinematic fit for particles in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param fitterEngine which fitter engine to use? 'NewFitterGSL' or 'OPALFitterGSL'
    @param constraint       HardBeam or RecoilMass or Mass
    @param invMass       Invariant Mass (GeV)
    @param daughtersUpdate make copy of the daughters and update them after the vertex fit
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
    orca.param('addUnmeasuredPhoton', False)
    path.add_module(orca)


if __name__ == '__main__':
    from basf2.utils import pretty_print_module
    pretty_print_module(__name__, "kinfit")
