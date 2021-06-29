#!/usr/bin/env python3

import re

import basf2 as b2
import modularAnalysis as ma
import pdg

from ROOT import Belle2
Const = Belle2.Const

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
    Function to prepare standard pion lists, refer to stdCharged for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('pi', listtype, path)


def stdK(listtype=_defaultlist, path=None):
    """
    Function to prepare standard kaon lists, refer to stdCharged for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('K', listtype, path)


def stdPr(listtype=_defaultlist, path=None):
    """
    Function to prepare standard proton lists, refer to stdCharged for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('p', listtype, path)


def stdLep(pdgId, listtype, method, classification, path=None):
    """
    Function to prepare one of several standardized types of lepton (e, mu) lists:

    - 'UniformEff50' 50% lepton efficiency list, uniform in a given multi-dimensional parametrisation.
    - 'UniformEff90' 90% lepton efficiency list, uniform in a given multi-dimensional parametrisation.
    - 'UniformEff99' 99% lepton efficiency list, uniform in a given multi-dimensional parametrisation.

    Parameters:
        pdgId (int): the lepton pdg code.
        listtype (str): name of standard list.
        method (str): the PID method: 'likelihood' or 'bdt'.
        classification (str): the type of classifier: 'binary' (one-vs-other) or 'global' (one-vs-all).
        path (basf2.Path): modules are added to this path.
    """

    std_lepton_list_names = (
        "UniformEff50",
        "UniformEff90",
        "UniformEff99",
    )

    available_methods = ("likelihood", "bdt")
    available_classificators = ("global", "binary")

    # We stick to positive pdgId by convention.
    # Anyway, the particle list will be filled for anti-particles too.
    pdgId = abs(pdgId)

    if listtype not in std_lepton_list_names:
        b2.B2ERROR("The requested lepton list is not defined. Please refer to the stdLep and stdCharged documentation.")
        return

    if pdgId not in (Const.electron.getPDGCode(), Const.muon.getPDGCode()):
        b2.B2ERROR(f"{pdgId} is not that of a light charged lepton.")
        return

    if method not in available_methods:
        b2.B2ERROR(f"method: {method}. Must be any of: {available_methods}.")
        return
    if classification not in available_classificators:
        b2.B2ERROR(f"classification: {classification}. Must be any of: {available_classificators}.")
        return

    pid_variables = {
        "likelihood": {
            "global": "electronID" if pdgId == Const.electron.getPDGCode() else "muonID",
            "binary": f"binaryPID({pdgId}, 211)"
        },
        "bdt": {
            "global": f"pidChargedBDTScore({pdgId}, ALL)",
            "binary": f"pidPairChargedBDTScore({pdgId}, 211, ALL)"
        }
    }

    # Start creating the particle list, w/o any selection.
    plistname = f"{pdg.to_name(pdgId)}:{listtype}"
    ma.fillParticleList(plistname, "", path=path)

    # The PID variable name, as it appears in the VariableManager.
    pid_var = pid_variables[method][classification]

    # Remove non-alphanumeric chars from the variable name, and strip last "_" if present.
    # This is needed to match the name of the payload in the CDB.
    pid_var_stripped = re.sub(r"[\W]+", "_", pid_var).rstrip("_")

    # The names of the payloads w/ efficiency and mis-id corrections.
    payload_eff = f"ParticleReweighting:{pid_var_stripped}_eff_combination_{listtype}"
    payload_misid_pi = f"ParticleReweighting:{pid_var_stripped}_misid_pi_combination_{listtype}"
    payload_misid_K = f"ParticleReweighting:{pid_var_stripped}_misid_K_combination_{listtype}"

    # Configure weighting module(s).
    reweighter_eff = path.add_module("ParticleWeighting",
                                     particleList=plistname,
                                     tableName=payload_eff)
    reweighter.set_name(f"ParticleWeighting_eff_{plistname}")
    reweighter_misid_pi = path.add_module("ParticleWeighting",
                                          particleList=plistname,
                                          tableName=payload_misid_pi)
    reweighter.set_name(f"ParticleWeighting_misid_pi_{plistname}")
    reweighter_misid_K = path.add_module("ParticleWeighting",
                                         particleList=plistname,
                                         tableName=payload_misid_K)
    reweighter.set_name(f"ParticleWeighting_misid_K_{plistname}")

    # Apply the PID selection cut, which is read from the efficiency payload.
    cut = f"{pid_var} > extraInfo({payload_eff}_threshold)"
    ma.applyCuts(plistname, cut, path=path)


def stdE(listtype=_defaultlist, method=None, classification=None, path=None):
    """ Function to prepare one of several standardized types of electron lists.
    See the documentation of `stdLep` for details.

    It also accepts any of the standard definitions
    for the ``listtype`` parameter to fall back to the `stdCharged` behaviour:

    * 'all'
    * 'good'
    * 'loosepid'
    * 'loose'
    * 'higheff'
    * '95eff'
    * '90eff'
    * '85eff'
    """

    if listtype in _stdnames + _effnames:
        stdCharged("e", listtype, path)
        return

    stdLep(Const.electron.getPDGCode(), listtype, method, classification, path=path)


def stdMu(listtype=_defaultlist, method=None, classification=None, path=None):
    """ Function to prepare one of several standardized types of muon lists.
    See the documentation of `stdLep` for details.

    It also accepts any of the standard definitions
    for the ``listtype`` parameter to fall back to the `stdCharged` behaviour:

    * 'all'
    * 'good'
    * 'loosepid'
    * 'loose'
    * 'higheff'
    * '95eff'
    * '90eff'
    * '85eff'
    """

    if listtype in _stdnames + _effnames:
        stdCharged("mu", listtype, path)
        return

    stdLep(Const.muon.getPDGCode(), listtype, method, classification, path=path)


def stdMostLikely(pidPriors=None, suffix='', custom_cuts='', path=None):
    """
    Function to prepare most likely particle lists according to PID likelihood, refer to stdCharged for details

    @param pidPriors    list of 6 float numbers used to reweight PID likelihoods
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
