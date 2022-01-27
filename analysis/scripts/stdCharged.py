#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import re

import basf2 as b2
import modularAnalysis as ma
from variables import variables
import pdg

from ROOT import Belle2
Const = Belle2.Const
_TrainingMode = Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode


# define arrays to interpret cut matrix
_chargednames = ['pi', 'K', 'p', 'e', 'mu']
_pidnames = ['pionID', 'kaonID', 'protonID', 'electronID', 'muonID']
_stdnames = ['all', 'loose', 'loosepid', 'good', 'higheff']
_effnames = ['95eff', '90eff', '85eff']
# default particle list for stdPi() and similar functions
_defaultlist = 'good'
_mostLikelyList = 'mostlikely'


def _stdChargedEffCuts(particletype, listtype):
    """
    Provides the PID cut corresponding to a given efficiency percentile

    @param particletype  type of charged particle (pi, K, p, e, mu)
    @param listtype      efficiency percentile for the list (95eff, 90eff, 85eff)
    """

    particleindex = _chargednames.index(particletype)
    effindex = _effnames.index(listtype)

    # efficiency cuts = [.95,.90,.85] efficiency; values outside (0,1) mean the cut does not exist and an error will be thrown
    effcuts = [[0.001, 0.019, 0.098],
               [5e-6, 0.027, 0.167],
               [0.000, 0.043, 0.251],
               [0.093, 0.301, 0.709],
               [0.187, 0.418, 0.909]]
    #
    return effcuts[particleindex][effindex]


def stdCharged(particletype, listtype, path):
    """
    Function to prepare one of several standardized types of charged particle lists:
      - 'all' with no cuts on track
      - 'good' high purity lists for data studies
      - 'loosepid' loose selections for skimming, PID cut only
      - 'loose' loose selections for skimming
      - 'higheff' high efficiency list with loose global ID cut for data studies
      - 'mostlikely' list with the highest PID likelihood
    Also the following lists, which may or may not be available depending on the release
      - '99eff' with 99% selection efficiency (calculated for 1<p<4 GeV) and good track (MC only)
      - '95eff' with 95% selection efficiency (calculated for 1<p<4 GeV) and good track (MC only)
      - '90eff' with 90% selection efficiency (calculated for 1<p<4 GeV) and good track (MC only)
      - '85eff' with 85% selection efficiency (calculated for 1<p<4 GeV) and good track (MC only)

    @param particletype type of charged particle to make a list of
    @param listtype     name of standard list
    @param path         modules are added to this path
    """

    # basic quality cut strings
    trackQuality = 'thetaInCDCAcceptance and nCDCHits>20'
    ipCut = 'dr < 0.5 and abs(dz) < 2'
    goodTrack = trackQuality + ' and ' + ipCut

    if particletype not in _chargednames:
        b2.B2ERROR("The requested list is not a standard charged particle. Use one of pi, K, e, mu, p.")

    if listtype == 'all':
        ma.fillParticleList(particletype + '+:all', '', True, path=path)
    elif listtype == 'good':
        ma.fillParticleList(
            particletype + '+:good',
            _pidnames[_chargednames.index(particletype)] + ' > 0.5 and ' + goodTrack,
            True,
            path=path)
    elif listtype == 'loose':
        ma.fillParticleList(
            particletype + '+:loose',
            _pidnames[_chargednames.index(particletype)] + ' > 0.1 and ' + goodTrack,
            True,
            path=path)
    elif listtype == 'loosepid':
        ma.fillParticleList(
            particletype + '+:loosepid',
            _pidnames[_chargednames.index(particletype)] + ' > 0.1',
            True,
            path=path)
    elif listtype == 'higheff':
        ma.fillParticleList(
            particletype + '+:higheff',
            _pidnames[_chargednames.index(particletype)] + ' > 0.002 and ' + goodTrack,
            True,
            path=path)
    elif listtype not in _effnames:
        b2.B2ERROR("The requested list is not defined. Please refer to the stdCharged documentation.")
    else:
        pidcut = _stdChargedEffCuts(particletype, listtype)
        if 0.0 < pidcut < 1.0:
            ma.fillParticleList(
                particletype +
                '+:' +
                listtype,
                _pidnames[_chargednames.index(particletype)] +
                ' > ' +
                str(pidcut) +
                ' and ' +
                goodTrack,
                True,
                path=path)
        else:
            b2.B2ERROR('The requested standard particle list ' + particletype +
                       '+:' + listtype + ' is not available in this release.')


def stdPi(listtype=_defaultlist, path=None):
    """
    Function to prepare standard pion lists, refer to `stdCharged` for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('pi', listtype, path)


def stdK(listtype=_defaultlist, path=None):
    """
    Function to prepare standard kaon lists, refer to `stdCharged` for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('K', listtype, path)


def stdPr(listtype=_defaultlist, path=None):
    """
    Function to prepare standard proton lists, refer to `stdCharged` for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('p', listtype, path)


def stdLep(pdgId,
           working_point,
           method,
           classification,
           lid_weights_gt,
           release=None,
           listname=None,
           trainingModeMulticlass=_TrainingMode.c_Multiclass,
           trainingModeBinary=_TrainingMode.c_Classification,
           path=None):
    """
    Function to prepare one of several standardized types of lepton (:math:`e,\\mu`) lists, with the following working points:

    * 'FixedThresh05', PID cut of > 0.5 for each particle in the list.
    * 'FixedThresh09', PID cut of > 0.9 for each particle in the list.
    * 'FixedThresh095', PID cut of > 0.95 for each particle in the list.
    * 'UniformEff60' 60% lepton efficiency list, uniform in a given multi-dimensional parametrisation.
    * 'UniformEff70' 70% lepton efficiency list, uniform in a given multi-dimensional parametrisation.
    * 'UniformEff80' 80% lepton efficiency list, uniform in a given multi-dimensional parametrisation.
    * 'UniformEff90' 90% lepton efficiency list, uniform in a given multi-dimensional parametrisation.
    * 'UniformEff95' 95% lepton efficiency list, uniform in a given multi-dimensional parametrisation.

    The function will select particles according to the chosen ``working_point``, and decorate each candidate
    with the nominal Data/MC :math:`\\ell` ID efficiency and :math:`\\pi,K` fake rate
    correction factors and their stat, syst uncertainty, reading the info from the Conditions Database (CDB) according
    to the chosen input global tag (GT).

    .. note::
        Particles will *not* be selected if they are outside the Data/MC efficiency corrections' phase space coverage
        for the given working point.
        In fact, the threshold value for the PID cut in such cases is set to NaN.

    Parameters:
        pdgId (int): the lepton pdg code.
        working_point (str): name of the chosen working point that defines the content of the list. Choose among the above values.
        method (str): the PID method: 'likelihood' or 'bdt'.
        classification (str): the type of classifier: 'binary' (one-vs-pion) or 'global' (one-vs-all).
        lid_weights_gt (str): the name identifier of the global tag with the recommended Data/MC correction weights.
                              Please refer to the
                              `Lepton ID Confluence page <https://confluence.desy.de/display/BI/Lepton+ID+Performance>`_
                              for info about lepton ID recommendations.
        release (Optional[int]): the major release number associated to the recommended global tag chosen.
                                 If specified, this ensures the correct :math:`\\ell` ID variables are used.
                                 Please refer to the
                                 `Lepton ID Confluence page <https://confluence.desy.de/display/BI/Lepton+ID+Performance>`_
                                 for info about lepton identification variables.
        listname (Optional[str]): the name of the lepton list.
                                  By default, it is assigned as: '{lepton}-:{method}_{classification}_{working_point}'
        trainingModeMulticlass (Optional[``Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode``]): enum identifier
                               of the multi-class (global PID) training mode.
                               See `modularAnalysis.applyChargedPidMVA` docs for available options.
        trainingModeBinary (Optional[``Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode``]): enum identifier
                           of the classification (binary PID) training mode.
                           See `modularAnalysis.applyChargedPidMVA` docs for available options.
        path (basf2.Path): modules are added to this path.

    Returns:
        (str): the alias for the lepton ID variable.
    """

    b2.B2INFO(f"Prepending GT with LID corrections: {lid_weights_gt}")
    b2.conditions.prepend_globaltag(lid_weights_gt)

    working_points = (
        "FixedThresh05",
        "FixedThresh09",
        "FixedThresh095",
        "UniformEff60",
        "UniformEff70",
        "UniformEff80",
        "UniformEff90",
        "UniformEff95",
    )

    available_methods = ("likelihood", "bdt")
    available_classificators = ("global", "binary")

    if working_point not in working_points:
        b2.B2ERROR("The requested lepton list working point is not defined. \
                   Please refer to the stdLep and stdCharged documentation.")
        return None

    if method not in available_methods:
        b2.B2ERROR(f"method: {method}. Must be any of: {available_methods}.")
        return None

    if classification not in available_classificators:
        b2.B2ERROR(f"classification: {classification}. Must be any of: {available_classificators}.")
        return None

    # We stick to positive pdgId by convention.
    # Anyway, the particle list will be filled for anti-particles too.
    lepton = abs(pdgId)
    lepton_name = pdg.to_name(lepton)
    electron = Const.electron.getPDGCode()
    muon = Const.muon.getPDGCode()
    pion = Const.pion.getPDGCode()

    if lepton not in (electron, muon):
        b2.B2ERROR(f"{pdgId} is not that of a light charged lepton.")
        return None

    # Declare LID variables (as in the VariableManager), and aliases to match the naming scheme used in the payloads.
    pid_variables = {
        "likelihood": {
            "global": {
                "var": {
                    electron: "electronID",
                    muon: "muonID",
                },
                "alias": {
                    electron: "electronID",
                    muon: "muonID",
                }
            },
            "binary": {
                "var": {
                    electron: f"binaryPID({electron}, {pion})",
                    muon: f"binaryPID({muon}, {pion})",
                },
                "alias": {
                    electron: "binaryPID_e_pi",
                    muon: "binaryPID_mu_pi",
                }
            }
        },
        "bdt": {
            "global": {
                "var": {
                    lepton: f"pidChargedBDTScore({lepton}, ALL)",
                },
                "alias": {
                    lepton: re.sub(r"\W+", "", f"pidChargedBDTScore_{lepton_name}"),
                }
            },
            "binary": {
                "var": {
                    lepton: f"pidPairChargedBDTScore({lepton}, {pion}, ALL)",
                },
                "alias": {
                    lepton: re.sub(r"\W+", "", f"pidPairChargedBDTScore_{lepton_name}_pi"),
                }
            }
        }
    }

    # Depending on the release associated to the chosen LID recommendations GT,
    # some variable names and aliases may need to be reset.
    if int(release) == 5:
        pid_variables["likelihood"]["global"]["var"][electron] = "electronID_noSVD_noTOP"
        pid_variables["likelihood"]["global"]["alias"][electron] = "electronID_noSVD_noTOP"
        pid_variables["likelihood"]["binary"]["var"][electron] = f"binaryElectronID_noSVD_noTOP({pion})"
        pid_variables["likelihood"]["binary"]["alias"][electron] = "binaryElectronID_noSVD_noTOP_pi"
    if int(release) == 6:
        pid_variables["likelihood"]["global"]["var"][electron] = "electronID_noTOP"
        pid_variables["likelihood"]["global"]["alias"][electron] = "electronID_noTOP"
        pid_variables["likelihood"]["binary"]["var"][electron] = f"binaryElectronID_noTOP({pion})"
        pid_variables["likelihood"]["binary"]["alias"][electron] = "binaryElectronID_noTOP_pi"

    # Create the aliases.
    pid_var = pid_variables[method][classification]["var"][lepton]
    pid_alias = pid_variables[method][classification]["alias"][lepton]
    if pid_alias != pid_var:
        variables.addAlias(pid_alias, pid_var)

    # Start creating the particle list, w/o any selection.
    plistname = f"{lepton_name}:{method}_{classification}_{working_point}"
    if listname is not None:
        plistname = f"{lepton_name}:{listname}"

    ma.fillParticleList(plistname, "", path=path)

    # Here we must run the BDT if requested.
    if method == "bdt":
        if classification == "global":
            ma.applyChargedPidMVA(particleLists=[plistname],
                                  path=path,
                                  trainingMode=trainingModeMulticlass)
        elif classification == "binary":
            ma.applyChargedPidMVA(particleLists=[plistname],
                                  path=path,
                                  binaryHypoPDGCodes=(lepton, pion),
                                  trainingMode=trainingModeBinary)

    # The names of the payloads w/ efficiency and mis-id corrections.
    payload_eff = f"ParticleReweighting:{pid_alias}_eff_combination_{working_point}"
    payload_misid_pi = f"ParticleReweighting:{pid_alias}_misid_pi_combination_{working_point}"
    payload_misid_K = f"ParticleReweighting:{pid_alias}_misid_K_combination_{working_point}"

    # Configure weighting module(s).
    path.add_module("ParticleWeighting",
                    particleList=plistname,
                    tableName=payload_eff).set_name(f"ParticleWeighting_eff_{plistname}")
    path.add_module("ParticleWeighting",
                    particleList=plistname,
                    tableName=payload_misid_pi).set_name(f"ParticleWeighting_misid_pi_{plistname}")
    path.add_module("ParticleWeighting",
                    particleList=plistname,
                    tableName=payload_misid_K).set_name(f"ParticleWeighting_misid_K_{plistname}")

    # Apply the PID selection cut, which is read from the efficiency payload.
    # The '>=' handles extreme cases in which the variable and the threshold value are at a boundary of the PID variable range.
    cut = f"[{pid_alias} >= extraInfo({payload_eff}_threshold)]"
    ma.applyCuts(plistname, cut, path=path)

    # Define convenience aliases for the nominal weight and up/dn variations.
    aliases_to_var = {
        f"weight_{pid_alias}_eff_{working_point}": f"extraInfo({payload_eff}_data_MC_ratio)",
        f"weight_{pid_alias}_misid_pi_{working_point}": f"extraInfo({payload_misid_pi}_data_MC_ratio)",
        f"weight_{pid_alias}_misid_K_{working_point}": f"extraInfo({payload_misid_K}_data_MC_ratio)",
        # These aliases are *relative* variations, so they can be multiplied to the nominal
        # to get the varied value:
        #
        # w_rel_var_up = (1 + w_up/w_nominal)
        # w_rel_var_dn = (1 - w_dn/w_nominal)
        #
        # w_var_up = w_nominal * w_rel_var_up
        # w_var_dn = w_nominal * w_rel_var_dn
        f"weight_{pid_alias}_eff_{working_point}_rel_stat_up":
        f"formula(1+[extraInfo({payload_eff}_data_MC_uncertainty_stat_up)/extraInfo({payload_eff}_data_MC_ratio)])",
        f"weight_{pid_alias}_eff_{working_point}_rel_stat_dn":
        f"formula(1-[extraInfo({payload_eff}_data_MC_uncertainty_stat_dn)/extraInfo({payload_eff}_data_MC_ratio)])",
        f"weight_{pid_alias}_eff_{working_point}_rel_sys_up":
        f"formula(1+[extraInfo({payload_eff}_data_MC_uncertainty_sys_up)/extraInfo({payload_eff}_data_MC_ratio)])",
        f"weight_{pid_alias}_eff_{working_point}_rel_sys_dn":
        f"formula(1-[extraInfo({payload_eff}_data_MC_uncertainty_sys_dn)/extraInfo({payload_eff}_data_MC_ratio)])",
        f"weight_{pid_alias}_misid_pi_{working_point}_rel_stat_up":
        f"formula(1+[extraInfo({payload_misid_pi}_data_MC_uncertainty_stat_up)/extraInfo({payload_misid_pi}_data_MC_ratio)])",
        f"weight_{pid_alias}_misid_pi_{working_point}_rel_stat_dn":
        f"formula(1-[extraInfo({payload_misid_pi}_data_MC_uncertainty_stat_dn)/extraInfo({payload_misid_pi}_data_MC_ratio)])",
        f"weight_{pid_alias}_misid_pi_{working_point}_rel_sys_up":
        f"formula(1+[extraInfo({payload_misid_pi}_data_MC_uncertainty_sys_up)/extraInfo({payload_misid_pi}_data_MC_ratio)])",
        f"weight_{pid_alias}_misid_pi_{working_point}_rel_sys_dn":
        f"formula(1-[extraInfo({payload_misid_pi}_data_MC_uncertainty_sys_dn)/extraInfo({payload_misid_pi}_data_MC_ratio)])",
        f"weight_{pid_alias}_misid_K_{working_point}_rel_stat_up":
        f"formula(1+[extraInfo({payload_misid_K}_data_MC_uncertainty_stat_up)/extraInfo({payload_misid_K}_data_MC_ratio)])",
        f"weight_{pid_alias}_misid_K_{working_point}_rel_stat_dn":
        f"formula(1-[extraInfo({payload_misid_K}_data_MC_uncertainty_stat_dn)/extraInfo({payload_misid_K}_data_MC_ratio)])",
        f"weight_{pid_alias}_misid_K_{working_point}_rel_sys_up":
        f"formula(1+[extraInfo({payload_misid_K}_data_MC_uncertainty_sys_up)/extraInfo({payload_misid_K}_data_MC_ratio)])",
        f"weight_{pid_alias}_misid_K_{working_point}_rel_sys_dn":
        f"formula(1-[extraInfo({payload_misid_K}_data_MC_uncertainty_sys_dn)/extraInfo({payload_misid_K}_data_MC_ratio)])",
    }

    for alias, var in aliases_to_var.items():
        variables.addAlias(alias, var)

    return pid_alias


def stdE(working_point,
         method,
         classification,
         lid_weights_gt,
         release=None,
         listname=None,
         trainingModeMulticlass=_TrainingMode.c_Multiclass,
         trainingModeBinary=_TrainingMode.c_Classification,
         path=None):
    """ Function to prepare one of several standardized types of electron lists.
    See the documentation of `stdLep` for details.

    It also accepts any of the legacy definitions
    for the ``working_point`` parameter to fall back to the `stdCharged` behaviour:

    * 'all'
    * 'good'
    * 'loosepid'
    * 'loose'
    * 'higheff'
    * '95eff'
    * '90eff'
    * '85eff'

    Returns:
        (str): the alias for the electron ID variable.
    """

    if working_point in _stdnames + _effnames:
        stdCharged("e", working_point, path)
        return _pidnames[_chargednames.index("e")]

    return stdLep(Const.electron.getPDGCode(), working_point, method, classification, lid_weights_gt,
                  release=release,
                  listname=listname,
                  trainingModeMulticlass=trainingModeMulticlass,
                  trainingModeBinary=trainingModeBinary,
                  path=path)


def stdMu(working_point,
          method,
          classification,
          lid_weights_gt,
          release=None,
          listname=None,
          trainingModeMulticlass=_TrainingMode.c_Multiclass,
          trainingModeBinary=_TrainingMode.c_Classification,
          path=None):
    """ Function to prepare one of several standardized types of muon lists.
    See the documentation of `stdLep` for details.

    It also accepts any of the legacy definitions
    for the ``working_point`` parameter to fall back to the `stdCharged` behaviour:

    * 'all'
    * 'good'
    * 'loosepid'
    * 'loose'
    * 'higheff'
    * '95eff'
    * '90eff'
    * '85eff'

    Returns:
        (str): the alias for the muon ID variable.
    """

    if working_point in _stdnames + _effnames:
        stdCharged("mu", working_point, path)
        return _pidnames[_chargednames.index("mu")]

    return stdLep(Const.muon.getPDGCode(), working_point, method, classification, lid_weights_gt,
                  release=release,
                  listname=listname,
                  trainingModeMulticlass=trainingModeMulticlass,
                  trainingModeBinary=trainingModeBinary,
                  path=path)


def stdMostLikely(pidPriors=None, suffix='', custom_cuts='', path=None):
    """
    Function to prepare most likely particle lists according to PID likelihood, refer to stdCharged for details

    @param pidPriors    list of 6 float numbers used to reweight PID likelihoods, for e, mu, pi, K, p and d
    @param suffix       string added to the end of particle list names
    @param custom_cuts  custom selection cut string, if empty, standard track quality cuts will be applied
    @param path         modules are added to this path
    """
    # Here we need basic track quality cuts to be applied,
    # otherwise, we get a lot of badly reconstructed particles,
    # which will end up filled as a random type
    args = ''
    if pidPriors is not None:
        args = str(pidPriors)[1:-1]  # remove brackets
    trackQuality = 'thetaInCDCAcceptance and nCDCHits>20'
    if custom_cuts != '':
        trackQuality = custom_cuts
    for name in _chargednames:
        ma.fillParticleList(f'{name}+:{_mostLikelyList}{suffix}',
                            f'pidIsMostLikely({args}) > 0 and {trackQuality}', True, path=path)
