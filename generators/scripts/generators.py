##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
This module contains convenience functions to setup most commonly used physics
generators correctly with their default settings. More information can be found
in `BELLE2-NOTE-PH-2015-006`_

.. _BELLE2-NOTE-PH-2015-006: https://docs.belle2.org/record/282
'''

import basf2 as b2
import pdg


def get_default_decayfile():
    """Return the default DECAY.dec for Belle2"""
    return b2.find_file("decfiles/dec/DECAY_BELLE2.DEC")


def add_generator_preselection(
        path,
        emptypath,
        nChargedMin=0,
        nChargedMax=999,
        MinChargedP=-1.0,
        MinChargedPt=-1.0,
        MinChargedTheta=0.0,
        MaxChargedTheta=180.0,
        nPhotonMin=0,
        nPhotonMax=999,
        MinPhotonEnergy=-1,
        MinPhotonTheta=0.0,
        MaxPhotonTheta=180.0,
        applyInCMS=False,
        stableParticles=False):
    """
        Adds generator preselection.
        Should be added to the path after the generator.add_abc_generator but before simulation.add_simulation modules
        It uses all particles from the event generator (i.e. primary, non-virtual, non-initial particles).
        It checks if the required conditions are fulfilled.
        If not, the events are given to the emptypath.
        The main use case is a reduction of simulation time.
        Note that you have to multiply the generated cross section by the retention fraction of the preselection.

        Parameters:
            path (basf2.Path): path where the generator should be added
            emptypath (basf2.Path): path where the skipped events are given to
            nChargedMin (int): minimum number of charged particles
            nChargedMax (int): maximum number of charged particles
            MinChargedP (float): minimum charged momentum [GeV]
            MinChargedPt (float): minimum charged transverse momentum (pt) [GeV]
            MinChargedTheta (float): minimum polar angle of charged particle [deg]
            MaxChargedTheta (float): maximum polar angle of charged particle [deg]
            nPhotonMin (int): minimum number of photons
            nPhotonMax (int): maximum number of photons
            MinPhotonEnergy (float): minimum photon energy [GeV]
            MinPhotonTheta (float): minimum polar angle of photon [deg]
            MaxPhotonTheta (float): maximum polar angle of photon [deg]
            applyInCMS (bool): if true apply the P,Pt,theta, and energy cuts in the center of mass frame
            stableParticles (bool): if true apply the selection criteria for stable particles in the generator
    """

    generatorpreselection = path.add_module('GeneratorPreselection',
                                            nChargedMin=nChargedMin,
                                            nChargedMax=nChargedMax,
                                            MinChargedP=MinChargedP,
                                            MinChargedPt=MinChargedPt,
                                            MinChargedTheta=MinChargedTheta,
                                            MaxChargedTheta=MaxChargedTheta,
                                            nPhotonMin=nPhotonMin,
                                            nPhotonMax=nPhotonMax,
                                            MinPhotonEnergy=MinPhotonEnergy,
                                            MinPhotonTheta=MinPhotonTheta,
                                            MaxPhotonTheta=MaxPhotonTheta,
                                            stableParticles=stableParticles
                                            )

    # empty path for unwanted events
    generatorpreselection.if_value('<11', emptypath)


def add_aafh_generator(
        path,
        finalstate='',
        preselection=False,
        enableTauDecays=True,
        minmass=0.5,
        subweights=[],
        maxsubweight=1,
        maxfinalweight=3.0,
        eventType=''):
    """
    Add the default two photon generator for four fermion final states

    Parameters:
        path (basf2.Path): path where the generator should be added
        finalstate (str): either "e+e-e+e-", "e+e-mu+mu-", "e+e-tau+tau-", "mu+mu-mu+mu-" or "mu+mu-tau+tau-"
        preselection (bool): if True, select events with at least one medium pt particle in the CDC acceptance
        enableTauDecays (bool): if True, allow tau leptons to decay (using EvtGen)
        minmass (float): minimum invariant mass
        subweights (list(float)): list of four or eight values (first four are interpreted as WAP, rest as WBP)
                                  which specify the relative weights for each of the four sub generators
        maxsubweight (float): maximum expected subgenerator weight for rejection scheme
        maxfinalweight (float): maximum expected final weight for rejection scheme
        eventType (str) : event type information
    """

    if finalstate == 'e+e-e+e-':
        aafh_mode = 5
        if not subweights:  # default subweights are for minmass=0.5
            aafh_subgeneratorWeights = [1.0, 7.986e+01, 5.798e+04, 3.898e+05, 1.0, 1.664e+00, 2.812e+00, 7.321e-01]
        else:
            aafh_subgeneratorWeights = subweights
        if abs(minmass - 0.5) > 0.01 and not subweights:
            b2.B2WARNING("add_aafh_generator: non default invariant mass cut without updated subweights requested!")
    elif finalstate == 'e+e-mu+mu-':
        aafh_mode = 3
        if not subweights:  # default subweights are for minmass=0.5
            aafh_subgeneratorWeights = [1.000e+00, 1.520e+01, 3.106e+03, 6.374e+03, 1.000e+00, 1.778e+00, 6.075e+00, 6.512e+00]
        else:
            aafh_subgeneratorWeights = subweights
        if abs(minmass - 0.5) > 0.01 and not subweights:
            b2.B2WARNING("add_aafh_generator: non default invariant mass cut without updated subweights requested!")
    elif finalstate == 'e+e-tau+tau-':
        aafh_mode = 4
        minmass = 0
        if not subweights:
            aafh_subgeneratorWeights = [1.000e+00, 2.214e+00, 1.202e+01, 1.536e+01, 1.000e+00, 1.664e+00, 1.680e+01, 6.934e+00]
        else:
            aafh_subgeneratorWeights = subweights
        if preselection:
            b2.B2WARNING(
                f"You requested a generator preselection for the final state {finalstate}: "
                "please consider to remove it, since the cross section is small.")
    elif finalstate == 'mu+mu-mu+mu-':
        aafh_mode = 2
        minmass = 0
        maxsubweight = 3
        if not subweights:
            aafh_subgeneratorWeights = [0.000e+00, 0.000e+00, 1.000e+00, 3.726e+00, 1.000e+00, 1.778e+00, 1.000e+00, 1.094e+00]
        else:
            aafh_subgeneratorWeights = subweights
        if preselection:
            b2.B2WARNING(
                f"You requested a generator preselection for the final state {finalstate}: "
                "please consider to remove it, since the cross section is small.")
    elif finalstate == 'mu+mu-tau+tau-':
        aafh_mode = 1
        minmass = 0
        maxsubweight = 3
        if not subweights:
            aafh_subgeneratorWeights = [0.000e+00, 0.000e+00, 1.000e+00, 1.715e+00, 1.000e+00, 1.778e+00, 1.000e+00, 6.257e-01]
        else:
            aafh_subgeneratorWeights = subweights
        if preselection:
            b2.B2WARNING(
                f"You requested a generator preselection for the final state {finalstate}: "
                "please consider to remove it, since the cross section is small.")
    elif finalstate == 'tau+tau-tau+tau-':
        b2.B2FATAL(f"AAFH is not able to generate the {finalstate} final state. Please use KoralW instead.")
    else:
        b2.B2FATAL(f"add_aafh_generator final state not supported: {finalstate}")

    aafh_maxSubgeneratorWeight = maxsubweight
    aafh_maxFinalWeight = maxfinalweight

    path.add_module(
        'AafhInput',
        mode=aafh_mode,
        rejection=2,
        maxSubgeneratorWeight=aafh_maxSubgeneratorWeight,
        maxFinalWeight=aafh_maxFinalWeight,
        subgeneratorWeights=aafh_subgeneratorWeights,
        suppressionLimits=[1e100] * 4,
        minMass=minmass,
        eventType=eventType,
    )

    if preselection:
        generator_emptypath = b2.create_path()
        add_generator_preselection(
            path=path,
            emptypath=generator_emptypath,
            nChargedMin=1,
            MinChargedPt=0.1,
            MinChargedTheta=17.0,
            MaxChargedTheta=150.0)

    if 'tau+tau-' in finalstate:
        if enableTauDecays:
            path.add_module('EvtGenDecay')
        else:
            b2.B2WARNING("The tau decays will not be generated.")


def add_kkmc_generator(path, finalstate='', signalconfigfile='', useTauolaBelle=False, tauinputfile='', eventType=''):
    """
    Add the default muon pair and tau pair generator KKMC.
    For tau decays, TauolaBelle and TauolaBelle2 are available.
    Signal events can be produced setting a configuration file. Please notice that the configuration files for
    TauolaBelle and TauolaBelle2 has a very different structure (see the examples below generators/examples).

    Parameters:
        path (basf2.Path): path where the generator should be added
        finalstate(str): either "mu-mu+" or "tau-tau+"
        signalconfigfile(str): File with configuration of the signal event to generate. It doesn't affect mu-mu+ decays.
        useTauolaBelle(bool): If true, tau decay is driven by TauolaBelle. Otherwise TauolaBelle2 is used.
                              It doesn't affect mu-mu+ decays.
        tauinputfile(str) : File to override KK2f_defaults. Only [sometimes] needed when tau decay is driven by TauolaBelle.
        eventType (str) : event type information
    """

    #: kkmc input file
    kkmc_inputfile = b2.find_file('data/generators/kkmc/tauola_bbb.input.dat')

    #: kkmc file that will hold cross section and other information
    kkmc_logfile = 'kkmc_tautau.txt'

    #: kkmc configuration file, should be fine as is
    kkmc_config = b2.find_file('data/generators/kkmc/KK2f_defaults.dat')

    #: tau config file (empty for generic mu-mu+ and tau-tau+ with TauolaBelle2)
    kkmc_tauconfigfile = ''

    if finalstate == 'tau+tau-':
        b2.B2WARNING("add_kkmc_generator: please set finalstate as 'tau-tau+'. 'tau+tau-' will be deprecated in the future"
                     " for consistency in the configuration files.")
        finalstate = 'tau-tau+'
    if finalstate == 'mu+mu-':
        b2.B2WARNING("add_kkmc_generator: please set finalstate as 'mu-mu+'. 'mu+mu-' will be deprecated in the future for"
                     " consistency in the configuration files.")
        finalstate = 'mu-mu+'

    if finalstate == 'tau-tau+':
        if useTauolaBelle:
            b2.B2INFO("Generating tau pair events with TauolaBelle")
            #: If TauolaBelle, the tau decay must be controlled by Pythia flags
            kkmc_inputfile = b2.find_file('data/generators/kkmc/tau.input.dat')
            kkmc_tauconfigfile = b2.find_file('data/generators/kkmc/tau_decaytable.dat')
        #: Check if there is a signal decfile provided by the user
        if not signalconfigfile == '':
            b2.B2INFO(f"Using config file defined by user: {signalconfigfile}")
            if useTauolaBelle:
                kkmc_tauconfigfile = b2.find_file(signalconfigfile)
            else:
                kkmc_inputfile = b2.find_file(signalconfigfile)
        #: Check if there is a tauinputfile to override KK2f_defaults. Only [sometimes] needed when using TauolaBelle.
        if not tauinputfile == '':
            kkmc_inputfile = b2.find_file(tauinputfile)

    elif finalstate == 'mu-mu+':
        kkmc_inputfile = b2.find_file('data/generators/kkmc/mu.input.dat')
        kkmc_logfile = 'kkmc_mumu.txt'

    else:
        b2.B2FATAL("add_kkmc_generator final state not supported: {}".format(finalstate))

    # use KKMC to generate lepton pairs
    path.add_module(
        'KKGenInput',
        tauinputFile=kkmc_inputfile,
        KKdefaultFile=kkmc_config,
        taudecaytableFile=kkmc_tauconfigfile,
        kkmcoutputfilename=kkmc_logfile,
        eventType=eventType,
    )


def add_evtgen_generator(path, finalstate='', signaldecfile=None, coherentMixing=True, parentParticle='Upsilon(4S)', eventType=''):
    """
    Add EvtGen for mixed and charged BB

    Parameters:
        path (basf2.Path): path where the generator should be added
        finalstate (str): Either "charged" for generation of generic B+/B-, "mixed" for generic B0/anti-B0, or "signal" for
                          generation of user-defined signal mode
        signaldecfile (str): path to decfile which defines the signal decay to be generated
                             (only needed if ``finalstate`` set to "signal")
        coherentMixing: Either True or False. Switches on or off the coherent decay of the B0-B0bar pair.
                        It should always be True, unless you are generating Y(5,6S) -> BBar. In the latter case,
                        setting it False solves the internal limitation of Evtgen that allows to make a
                        coherent decay only starting from the Y(4S).
        parentParticle (str): initial state (used only if it is not Upsilon(4S).
        eventType (str) : event type information
    """
    evtgen_userdecfile = b2.find_file('data/generators/evtgen/charged.dec')

    if parentParticle != 'Upsilon(3S)' and parentParticle != 'Upsilon(4S)'\
            and parentParticle != 'Upsilon(5S)' and parentParticle != 'Upsilon(6S)':
        b2.B2FATAL("add_evtgen_generator initial state not supported: {}".format(parentParticle))

    if finalstate == 'charged':
        pass
    elif finalstate == 'mixed':
        evtgen_userdecfile = b2.find_file('data/generators/evtgen/mixed.dec')
    elif finalstate == 'signal':
        evtgen_userdecfile = signaldecfile
    else:
        b2.B2FATAL("add_evtgen_generator final state not supported: {}".format(finalstate))

    if signaldecfile and finalstate in ['charged', 'mixed']:
        b2.B2WARNING("ignoring decfile: {}".format(signaldecfile))

    # use EvtGen
    if parentParticle == 'Upsilon(3S)':
        if finalstate != 'signal':
            b2.B2FATAL("add_evtgen_generator initial state {} is supported only with 'signal' final state".format(parentParticle))
        if coherentMixing:
            coherentMixing = False
            b2.B2WARNING("add_evtgen_generator initial state {} has no BB mixing, now switching coherentMixing OFF"
                         .format(parentParticle))

    if parentParticle == 'Upsilon(5S)':
        if finalstate != 'signal':
            b2.B2FATAL("add_evtgen_generator initial state {} is supported only with 'signal' final state".format(parentParticle))
        if coherentMixing:
            coherentMixing = False
            b2.B2WARNING(
                "add_evtgen_generator initial state {} is supported only with false coherentMixing, now switching it OFF"
                .format(parentParticle))
        pdg.load(b2.find_file('decfiles/dec/Y5S.pdl'))

    if parentParticle == 'Upsilon(6S)':
        if finalstate != 'signal':
            b2.B2FATAL("add_evtgen_generator initial state {} is supported only with 'signal' final state".format(parentParticle))
        if coherentMixing:
            coherentMixing = False
            b2.B2WARNING(
                "add_evtgen_generator initial state {} is supported only with false coherentMixing, now switching it OFF"
                .format(parentParticle))
        pdg.load(b2.find_file('decfiles/dec/Y6S.pdl'))

    path.add_module(
        'EvtGenInput',
        userDECFile=evtgen_userdecfile,
        CoherentMixing=coherentMixing,
        ParentParticle=parentParticle,
        eventType=eventType,
    )


def add_continuum_generator(path, finalstate, userdecfile='', *, skip_on_failure=True, eventType=''):
    """
    Add the default continuum generators KKMC + PYTHIA including their default decfiles and PYTHIA settings

    See Also:
        `add_inclusive_continuum_generator()` to add continuum generation with preselected particles

    Parameters:
        path (basf2.Path): path where the generator should be added
        finalstate (str): uubar, ddbar, ssbar, ccbar
        userdecfile (str): EvtGen decfile used for particle decays
        skip_on_failure (bool): If True stop event processing right after
            fragmentation fails. Otherwise continue normally
        eventType (str) : event type information
    """

    #: kkmc input file, one for each qqbar mode
    kkmc_inputfile = b2.find_file('data/generators/kkmc/uubar_nohadronization.input.dat')

    #: kkmc file that will hold cross section and other information
    kkmc_logfile = 'kkmc_uubar.txt'

    #: pythia configuration, different for ccbar
    pythia_config = b2.find_file('data/generators/modules/fragmentation/pythia_belle2.dat')

    #: user decay file
    decay_user = b2.find_file('data/generators/modules/fragmentation/dec_belle2_qqbar.dec')
    if userdecfile == '':
        pass
    else:
        b2.B2INFO('Replacing default user decfile: {}'.format(userdecfile))
        decay_user = userdecfile

    #: kkmc configuration file, should be fine as is
    kkmc_config = b2.find_file('data/generators/kkmc/KK2f_defaults.dat')

    #: global decay file
    decay_file = get_default_decayfile()

    if finalstate == 'uubar':
        pass
    elif finalstate == 'ddbar':
        kkmc_inputfile = b2.find_file('data/generators/kkmc/ddbar_nohadronization.input.dat')
        kkmc_logfile = 'kkmc_ddbar.txt'
    elif finalstate == 'ssbar':
        kkmc_inputfile = b2.find_file('data/generators/kkmc/ssbar_nohadronization.input.dat')
        kkmc_logfile = 'kkmc_ssbar.txt'
    elif finalstate == 'ccbar':
        kkmc_inputfile = b2.find_file('data/generators/kkmc/ccbar_nohadronization.input.dat')
        pythia_config = b2.find_file('data/generators/modules/fragmentation/pythia_belle2_charm.dat')
        kkmc_logfile = 'kkmc_ccbar.txt'
    else:
        b2.B2FATAL("add_continuum_generator final state not supported: {}".format(finalstate))

    # use KKMC to generate qqbar events (no fragmentation at this stage)
    path.add_module(
        'KKGenInput',
        tauinputFile=kkmc_inputfile,
        KKdefaultFile=kkmc_config,
        taudecaytableFile='',
        kkmcoutputfilename=kkmc_logfile,
        eventType=eventType,
    )

    # add the fragmentation module to fragment the generated quarks into hadrons
    # using PYTHIA8
    fragmentation = path.add_module(
        'Fragmentation',
        ParameterFile=pythia_config,
        ListPYTHIAEvent=0,
        UseEvtGen=1,
        DecFile=decay_file,
        UserDecFile=decay_user,
    )

    if skip_on_failure:
        # branch to an empty path if PYTHIA failed, this will change the number of events
        # but the file meta data will contain the total number of generated events
        generator_emptypath = b2.create_path()
        fragmentation.if_value('<1', generator_emptypath)


def add_inclusive_continuum_generator(path, finalstate, particles, userdecfile='',
                                      *, include_conjugates=True, max_iterations=100000, eventType=''):
    """
    Add continuum generation but require at least one of the given particles be
    present in the event.

    For example to only generate ccbar events which contain a "D*+" or an
    electron one could would use

    >>> add_inclusive_continuum_generator(path, "ccbar", ["D*+", 11])

    If you are unsure how the particles are named in Belle II please have a look
    at the ``b2help-particles`` executable or the `pdg` python module.

    See Also:
        `add_continuum_generator()` to add continuum generation without preselection

    Parameters:
        path (basf2.Path): path to which the generator should be added
        finalstate (str): uubar, ddbar, ssbar, ccbar
        particles (list): A list of particle names or pdg codes. An event is
           only accepted if at lease one of those particles appears in the event.
        userdecfile (str): EvtGen decfile used for particle decays
        include_conjugates (bool): If True (default) accept the event also if a
            charge conjugate of the given particles is found
        max_iterations (int): maximum tries per event to generate the requested
            particle. If exceeded processing will be stopped with a
            `FATAL <LogLevel.FATAL>` error so for rare particles one might need a
            larger number.
        eventType (str) : event type information
    """
    loop_path = b2.create_path()
    # we might run this more than once so make sure we remove any particles
    # before generating new ones
    loop_path.add_module("PruneDataStore", keepMatchedEntries=False, matchEntries=["MCParticles", "EventExtraInfo"])
    # add the generator but make sure it doesn't stop processing on
    # fragmentation failure as is this currently not supported by do_while
    add_continuum_generator(loop_path, finalstate, userdecfile, skip_on_failure=False, eventType=eventType)
    # check for the particles we want
    loop_path.add_module("InclusiveParticleChecker", particles=particles, includeConjugates=include_conjugates)
    # Done, add this to the path and iterate it until we found our particle
    path.do_while(loop_path, max_iterations=max_iterations)


def add_bhwide_generator(path, minangle=0.5):
    """
    Add the high precision QED generator BHWIDE to the path. Settings are the default L1/HLT study settings
    with a cross section of about 124000 nb (!)

    Parameters:
        path (basf2.Path): path where the generator should be added
        minangle (float): minimum angle of the outgoing electron/positron in the CMS in degrees
    """

    if minangle < 0.0 or minangle > 180.0:
        b2.B2FATAL("add_bhwide_generator minimum angle too small (<0.0) or too large (>180): {}".format(minangle))

    bhwide = path.add_module("BHWideInput")
    bhwide.param('ScatteringAngleRangePositron', [minangle, 180.0 - minangle])
    bhwide.param('ScatteringAngleRangeElectron', [minangle, 180.0 - minangle])
    bhwide.param('MaxAcollinearity', 180.0)
    bhwide.param('MinEnergy', 0.10)
    bhwide.param('VacuumPolarization', 'burkhardt')
    bhwide.param('WeakCorrections', True)
    bhwide.param('WtMax', 3.0)


def add_babayaganlo_generator(
        path,
        finalstate='',
        minenergy=0.01,
        minangle=10.0,
        fmax=-1.0,
        generateInECLAcceptance=False,
        eventType=''):
    '''
    Add the high precision QED generator BabaYaga@NLO to the path.

    Parameters:
        path (basf2.Path): path where the generator should be added.
        finalstate (str): ee (e+e-), mm(mu+mu-) or gg (gammagamma).
        minenergy (float): minimum particle (leptons for 'ee' or 'mm', photons for 'gg') energy in GeV.
        minangle (float): angular range from minangle to 180-minangle for primary particles (in degrees).
        fmax (float): maximum of differential cross section weight. This parameter should be set only by experts.
        generateInECLAcceptance (bool): if True, the GeneratorPreselection module is used to select only events
          with both the primary particles within the ECL acceptance.
        eventType (str) : event type information
    '''

    babayaganlo = path.add_module('BabayagaNLOInput')
    babayaganlo.param('eventType', eventType)
    if not (fmax == -1.0):
        b2.B2WARNING(f'The BabayagaNLOInput parameter "FMax" will be set to {fmax} instead to the default value (-1.0). '
                     'Please do not do this, unless you are extremely sure about this choice.')

    if finalstate != 'ee' and finalstate != 'gg' and finalstate != 'mm':
        b2.B2FATAL(f'add_babayaganlo_generator final state not supported: {finalstate}')

    babayaganlo.param('FinalState', finalstate)
    babayaganlo.param('ScatteringAngleRange', [minangle, 180.0 - minangle])
    babayaganlo.param('MinEnergy', minenergy)
    babayaganlo.param('FMax', fmax)

    if generateInECLAcceptance:
        b2.B2INFO(f'The final state {finalstate} is preselected requiring both primary particles within the ECL acceptance.')
        emptypath = b2.Path()
        add_generator_preselection(path=path,
                                   emptypath=emptypath,
                                   applyInCMS=False)

        b2.set_module_parameters(path=path,
                                 name='GeneratorPreselection',
                                 nChargedMin=2,
                                 MinChargedTheta=12.4,
                                 MaxChargedTheta=155.1)


def add_phokhara_generator(path, finalstate='', eventType=''):
    """
    Add the high precision QED generator PHOKHARA to the path. Almost full
    acceptance settings for photons and hadrons/muons.

    Parameters:
        path (basf2.Path): path where the generator should be added
        finalstate (str): One of the following final states: "mu+mu-", "pi+pi-", "pi+pi-pi0", "pi+pi-pi+pi-" (or "2(pi+pi-)"),
          "pi+pi-pi0pi0" or ("pi+pi-2pi0"), "pi+pi-eta", "K+K-", "K0K0bar", "ppbar", "n0n0bar" or "Lambda0Lambda0bar"
        eventType (str) : event type information
    """

    if finalstate == 'mu+mu-':
        path.add_module(
            'PhokharaInput',
            FinalState=0,        # mu+mu-
            LO=0, NLO=1, QED=0,  # use full two loop corrections
            MinInvMassHadrons=0.,
            eventType=eventType,
        ).set_name('PHOKHARA_mumuISR')

    elif finalstate == 'pi+pi-':
        path.add_module(
            'PhokharaInput',
            FinalState=1,        # pi+pi-
            LO=0, NLO=1, QED=0,  # use full two loop corrections
            MinInvMassHadrons=0.,
            eventType=eventType,
        ).set_name('PHOKHARA_pipiISR')

    elif finalstate == 'pi+pi-pi0':
        path.add_module(
            'PhokharaInput',
            FinalState=8,        # pi+pi-pi0
            LO=0, NLO=1, QED=0,  # use full two loop corrections
            MinInvMassHadrons=0.,
            eventType=eventType,
        ).set_name('PHOKHARA_pipipi0ISR')

    elif finalstate == 'pi+pi-pi+pi-' or finalstate == '2(pi+pi-)':
        path.add_module(
            'PhokharaInput',
            FinalState=3,        # 2(pi+pi-)
            LO=0, NLO=1, QED=0,  # use full two loop corrections
            MinInvMassHadrons=0.,
            eventType=eventType,
        ).set_name('PHOKHARA_pipipipiISR')

    elif finalstate == 'pi+pi-pi0pi0' or finalstate == 'pi+pi-2pi0':
        path.add_module(
            'PhokharaInput',
            FinalState=2,        # pi+pi-2pi0
            LO=0, NLO=1, QED=0,  # use full two loop corrections
            MinInvMassHadrons=0.,
            eventType=eventType,
        ).set_name('PHOKHARA_pipipi0pi0ISR')

    elif finalstate == 'pi+pi-eta':
        path.add_module(
            'PhokharaInput',
            FinalState=10,       # pi+pi-eta
            LO=0, NLO=1, QED=0,  # use full two loop corrections
            MinInvMassHadrons=0.,
            eventType=eventType,
        ).set_name('PHOKHARA_pipietaISR')

    elif finalstate == 'K+K-':
        path.add_module(
            'PhokharaInput',
            FinalState=6,        # K+K-
            LO=0, NLO=1, QED=0,  # use full two loop corrections
            MinInvMassHadrons=0.,
            eventType=eventType,
        ).set_name('PHOKHARA_KKISR')

    elif finalstate == 'K0K0bar':
        path.add_module(
            'PhokharaInput',
            FinalState=7,        # K0K0bar
            LO=0, NLO=1, QED=0,  # use full two loop corrections
            MinInvMassHadrons=0.,
            eventType=eventType,
        ).set_name('PHOKHARA_K0K0barISR')

    elif finalstate == 'ppbar':
        path.add_module(
            'PhokharaInput',
            FinalState=4,        # ppbar
            LO=0, NLO=1, QED=0,  # use full two loop corrections
            MinInvMassHadrons=0.,
            eventType=eventType,
        ).set_name('PHOKHARA_ppbarISR')

    elif finalstate == 'n0n0bar':
        path.add_module(
            'PhokharaInput',
            FinalState=5,        # n0n0bar
            LO=0, NLO=1, QED=0,  # use full two loop corrections
            MinInvMassHadrons=0.,
            eventType=eventType,
        ).set_name('PHOKHARA_n0n0barISR')

    elif finalstate == 'Lambda0Lambda0bar':
        path.add_module(
            'PhokharaInput',
            FinalState=9,        # Lambda0Lambda0bar
            LO=0, NLO=1, QED=0,  # use full two loop corrections
            MinInvMassHadrons=0.,
            eventType=eventType,
        ).set_name('PHOKHARA_Lambda0Lambda0barISR')

    else:
        b2.B2FATAL(f"add_phokhara_generator final state not supported: {finalstate}")


def add_phokhara_evtgen_combination(
        path, final_state_particles, user_decay_file,
        beam_energy_spread=True, isr_events=False, min_inv_mass_vpho=0.0,
        max_inv_mass_vpho=0.0, eventType=''):
    """
    Add combination of PHOKHARA and EvtGen to the path. Phokhara is
    acting as ISR generator by generating e+ e- -> mu+ mu-, the muon pair is
    then replaced by a virtual photon. Finally, the virtual photon is
    decayed by EvtGen.

    Parameters:
        path (basf2.Path): Path where the generator should be added.
        final_state_particles (list): List of final-state particles of
            the virtual-photon decay. It is necessary to define the correct
            mass threshold in PHOKHARA. For example, for the process
            e+ e- -> J/psi eta_c, the list should be ['J/psi', 'eta_c'];
            it does not depend on subsequent J/psi or eta_c decays.
        user_decay_file (str): Name of EvtGen user decay file. The initial
            particle must be the virtual photon (vpho).
        beam_energy_spread (bool): Whether beam-energy spread should be
            simulated
        isr_events (bool): If true, then PHOKHARA is used with ph0 (LO in
            basf2) = 0, i.e. generation of processes with one photon.
        min_inv_mass_hadrons(float): Minimum invariant mass of the virtual
            photon. This parameter is used only if isr_events is true,
            otherwise the minimum mass is computed from the masses of
            the final-state particles.
        max_inv_mass_hadrons(float): Maximum invariant mass of the virtual
            photon. This parameter is used only if isr_events is true,
            otherwise the maximum mass is not restricted.
        eventType (str) : event type information
    """

    import pdg

    phokhara = path.add_module('PhokharaInput')
    phokhara.param('eventType', eventType)

    # Generate muons and replace them by a virtual photon.
    phokhara.param('FinalState', 0)
    phokhara.param('ReplaceMuonsByVirtualPhoton', True)

    # Simulate beam-energy spread. This performs initialization for every
    # event, and, thus, very slow, but usually necessary except for testing.
    phokhara.param('BeamEnergySpread', beam_energy_spread)

    # Soft photon cutoff.
    phokhara.param('Epsilon', 0.0001)

    # Maximum search.
    phokhara.param('SearchMax', 5000)

    # Number of generation attempts for event.
    phokhara.param('nMaxTrials', 25000)

    # Use NNLO.
    if isr_events:
        phokhara.param('LO', 0)
    else:
        phokhara.param('LO', 1)
    phokhara.param('NLO', 1)

    # Use ISR only.
    phokhara.param('QED', 0)

    # No interference.
    phokhara.param('IFSNLO', 0)

    # Vacuum polarization by Fred Jegerlehner.
    phokhara.param('Alpha', 1)

    # Do not include narrow resonances.
    phokhara.param('NarrowRes', 0)

    # Angular ragnes.
    phokhara.param('ScatteringAngleRangePhoton', [0., 180.])
    phokhara.param('ScatteringAngleRangeFinalStates', [0., 180.])

    # Minimal invariant mass of the muons and tagged photon combination.
    phokhara.param('MinInvMassHadronsGamma', 0.)

    if isr_events:
        # Minimum squared invariant mass of muons (final state).
        phokhara.param('MinInvMassHadrons',
                       min_inv_mass_vpho * min_inv_mass_vpho)
        # Maximum squared invariant mass of muons (final state).
        phokhara.param('MaxInvMassHadrons',
                       max_inv_mass_vpho * max_inv_mass_vpho)
    else:
        # Minimum squared invariant mass of muons (final state).
        # Force application of this cut.
        mass = 0
        for particle in final_state_particles:
            p = pdg.get(particle)
            mass = mass + p.Mass()
        phokhara.param('MinInvMassHadrons', mass * mass)
        phokhara.param('ForceMinInvMassHadronsCut', True)
        # Maximum squared invariant mass of muons (final state).
        # It is set to a large value, resulting in unrestricted mass.
        phokhara.param('MaxInvMassHadrons', 200.0)

    # Minimal photon energy.
    phokhara.param('MinEnergyGamma', 0.01)

    # EvtGen.
    evtgen_decay = path.add_module('EvtGenDecay')
    evtgen_decay.param('UserDecFile', user_decay_file)


def add_koralw_generator(path, finalstate='', enableTauDecays=True, eventType=''):
    """
    Add KoralW generator for radiative four fermion final states (only four leptons final states are currently supported).

    Parameters:
        path (basf2.Path): path where the generator should be added
        finalstate (str): either 'e+e-e+e-', 'e+e-mu+mu-', 'e+e-tau+tau-', 'mu+mu-mu+mu-', 'mu+mu-tau+tau-' or 'tau+tau-tau+tau-'
        enableTauDecays (bool): if True, allow tau leptons to decay (using EvtGen)
        eventType (str) : event type information
    """

    decayFile = ''
    if finalstate == 'e+e-e+e-':
        decayFile = b2.find_file('data/generators/koralw/KoralW_eeee.data')
    elif finalstate == 'e+e-mu+mu-':
        decayFile = b2.find_file('data/generators/koralw/KoralW_eeMuMu.data')
    elif finalstate == 'e+e-tau+tau-':
        decayFile = b2.find_file('data/generators/koralw/KoralW_eeTauTau.data')
    elif finalstate == 'mu+mu-mu+mu-':
        decayFile = b2.find_file('data/generators/koralw/KoralW_MuMuMuMu.data')
    elif finalstate == 'mu+mu-tau+tau-':
        decayFile = b2.find_file('data/generators/koralw/KoralW_MuMuTauTau.data')
    elif finalstate == 'tau+tau-tau+tau-':
        decayFile = b2.find_file('data/generators/koralw/KoralW_TauTauTauTau.data')
    else:
        b2.B2FATAL(f'add_koralw_generator final state not supported: {finalstate}')

    path.add_module('KoralWInput',
                    UserDataFile=decayFile,
                    eventType=eventType)

    if 'tau+tau-' in finalstate:
        if enableTauDecays:
            path.add_module('EvtGenDecay')
        else:
            b2.B2WARNING('The tau decays will not be generated.')


def add_cosmics_generator(path, components=None,
                          global_box_size=None, accept_box=None, keep_box=None,
                          geometry_xml_file='geometry/Beast2_phase2.xml',
                          cosmics_data_dir='data/generators/modules/cryinput/',
                          setup_file='generators/scripts/cry.setup',
                          data_taking_period='gcr2017', top_in_counter=False):
    """
    Add the cosmics generator CRY with the default parameters to the path.

    Warning:
        Please remember to also change the reconstruction accordingly, if you
        set "special" parameters here!

    Parameters:
        path (basf2.Path): path where the generator should be added
        components (list(str)): list of geometry components to add in the
            geometry module, or None for all components.
        global_box_size (tuple(float, float, float)): sets global length, width
            and height (in meters) in which to generate.
            Default is ``[100, 100, 100]``
        accept_box (tuple(float, float, float)): sets the size of the accept box in meter.
            As a default it is set to ``[8.0, 8.0, 8.0]`` (the Belle II detector size).
        keep_box (tuple(float, float, float)): sets the size of the keep box (keep box >= accept box).
        geometry_xml_file (str): Name of the xml file to use for the geometry.
        cosmics_data_dir (str): parameter CosmicDataDir for the cry module (absolute or relative to the basf2 repo).
        setup_file (str): location of the cry.setup file (absolute or relative to the basf2 repo)
        data_taking_period (str): The cosmics generation will be added using the
            parameters, that where used in this period of data taking. The
            periods can be found in ``cdc/cr/__init__.py``.
        top_in_counter (bool): time of propagation from the hit point to the PMT in the trigger counter is subtracted
            (assuming PMT is put at -z of the counter).
    """

    b2.B2FATAL('''The function "add_cosmics_generator()" is outdated and it is currently not working: please replace

  add_cosmics_generator(path=path)

with

  path.add_module('CRYInput')

in your steering file (the module parameter "acceptance" has to be set, see the module docummentation).''')

    import cdc.cr as cosmics_setup

    if global_box_size is None:
        global_box_size = [100, 100, 100]
    if accept_box is None:
        accept_box = [8, 8, 8]
    if keep_box is None:
        keep_box = [8, 8, 8]

    cosmics_setup.set_cdc_cr_parameters(data_taking_period)

    if cosmics_setup.cosmics_period == "201607":
        b2.B2FATAL("The data taking period 201607 is very special (geometry setup, PMTs etc). This is not handled "
                   "by this script! Please ask the CDC group, if you want to simulate this.")

    if 'Gearbox' not in path:
        override = [("/Global/length", str(global_box_size[0]), "m"),
                    ("/Global/width", str(global_box_size[1]), "m"),
                    ("/Global/height", str(global_box_size[2]), "m")]

        if cosmics_setup.globalPhi:
            override += [("/DetectorComponent[@name='CDC']//GlobalPhiRotation", str(cosmics_setup.globalPhi), "deg")]

        path.add_module('Gearbox', override=override, fileName=geometry_xml_file,)

    # detector geometry
    if 'Geometry' not in path:
        geometry = path.add_module('Geometry')
        if components:
            geometry.param('components', components)

    cry = path.add_module('CRYInput')

    # cosmic data input
    cry.param('CosmicDataDir', b2.find_file(cosmics_data_dir))

    # user input file
    cry.param('SetupFile', b2.find_file(setup_file))

    # acceptance half-lengths - at least one particle has to enter that box to use that event
    cry.param('acceptLength', accept_box[0])
    cry.param('acceptWidth', accept_box[1])
    cry.param('acceptHeight', accept_box[2])
    cry.param('maxTrials', 100000)

    # keep half-lengths - all particles that do not enter the box are removed (keep box >= accept box)
    # default was 6.0
    cry.param('keepLength', keep_box[0])
    cry.param('keepWidth', keep_box[1])
    cry.param('keepHeight', keep_box[2])

    # minimal kinetic energy - all particles below that energy are ignored
    cry.param('kineticEnergyThreshold', 0.01)

    # TODO: I still do not fully understand, when the cosmics selector is needed and when not
    if cosmics_setup.cosmics_period not in ["normal", "gcr2017"]:
        # Selector module.
        cosmics_selector = b2.register_module('CDCCosmicSelector',
                                              lOfCounter=cosmics_setup.lengthOfCounter,
                                              wOfCounter=cosmics_setup.widthOfCounter,
                                              xOfCounter=cosmics_setup.triggerPos[0],
                                              yOfCounter=cosmics_setup.triggerPos[1],
                                              zOfCounter=cosmics_setup.triggerPos[2],
                                              phiOfCounter=0.,
                                              TOP=top_in_counter,
                                              propSpeed=cosmics_setup.lightPropSpeed,
                                              TOF=1,
                                              cryGenerator=True
                                              )

        path.add_module(cosmics_selector)

        empty_path = b2.create_path()
        cosmics_selector.if_false(empty_path)


def add_treps_generator(path, finalstate='', useDiscreteAndSortedW=False, eventType=''):
    """
    Add TREPS generator to produce hadronic two-photon processes.

    Parameters:
        path (basf2.Path):           path where the generator should be added
        finalstate(str):             "e+e-pi+pi-", "e+e-K+K-" or "e+e-ppbar"
        useDiscreteAndSortedW(bool): if True, wListTableFile is used for discrete and sorted W. evtNumList must be set proper value.
        eventType (str) : event type information
    """

    if finalstate == 'e+e-pi+pi-':
        parameterFile = b2.find_file('generators/treps/data/parameterFiles/treps_par_pipi.dat')
        differentialCrossSectionFile = b2.find_file('generators/treps/data/differentialCrossSectionFiles/pipidcs.dat')
        wListTableFile = b2.find_file('generators/treps/data/wListFiles/wlist_table_pipi.dat')
    elif finalstate == 'e+e-K+K-':
        parameterFile = b2.find_file('generators/treps/data/parameterFiles/treps_par_kk.dat')
        differentialCrossSectionFile = b2.find_file('generators/treps/data/differentialCrossSectionFiles/kkdcs.dat')
        wListTableFile = b2.find_file('generators/treps/data/wListFiles/wlist_table_kk.dat')
    elif finalstate == 'e+e-ppbar':
        parameterFile = b2.find_file('generators/treps/data/parameterFiles/treps_par_ppbar.dat')
        differentialCrossSectionFile = b2.find_file('generators/treps/data/differentialCrossSectionFiles/ppbardcs.dat')
        wListTableFile = b2.find_file('generators/treps/data/wListFiles/wlist_table_ppbar.dat')
    else:
        b2.B2FATAL("add_treps_generator final state not supported: {}".format(finalstate))

    # use TREPS to generate two-photon events.
    path.add_module(
        'TrepsInput',
        ParameterFile=parameterFile,
        DifferentialCrossSectionFile=differentialCrossSectionFile,
        WListTableFile=wListTableFile,
        UseDiscreteAndSortedW=useDiscreteAndSortedW,
        MaximalQ2=1.0,
        MaximalAbsCosTheta=1.01,
        ApplyCosThetaCutCharged=True,
        MinimalTransverseMomentum=0,
        ApplyTransverseMomentumCutCharged=True,
        eventType=eventType,
    )
