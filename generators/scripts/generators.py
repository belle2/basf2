"""
This module contains convenience functions to setup most commonly used physics
generators correctly with their default settings. More information can be found
in `BELLE2-NOTE-PH-2015-006`_

Contact: Torben Ferber (ferber@physics.ubc.ca)

.. _BELLE2-NOTE-PH-2015-006: https://docs.belle2.org/record/282
"""

from basf2 import *
from ROOT import Belle2
import os


def get_default_decayfile():
    """Return the default DECAY.dec for Belle2"""
    return Belle2.FileSystem.findFile("generators/evtgen/decayfiles/DECAY_BELLE2.DEC")


def add_aafh_generator(path, finalstate='', preselection=False, minmass=0.5, subweights=[], maxsubweight=1, maxfinalweight=3.0):
    """
    Add the default two photon generator for four fermion final states

    Parameters:
        path (basf2.Path): path where the generator should be added
        finalstate (str): either "e+e-e+e-" or "e+e-mu+mu-"
        preselection (bool): if True, select events with at least one medium pt particle in the CDC acceptance
    """

    aafh = register_module('AafhInput')
    aafh_mode = 5
    if not subweights:  # default subweights are for minmass=0.5
        aafh_subgeneratorWeights = [1.0, 7.986e+01, 5.798e+04, 3.898e+05, 1.0, 1.664e+00, 2.812e+00, 7.321e-01]
    else:
        aafh_subgeneratorWeights = subweights
    aafh_maxSubgeneratorWeight = maxsubweight
    aafh_maxFinalWeight = maxfinalweight

    if abs(minmass - 0.5) > 0.01 and not subweights:
        B2WARNING("add_aafh_generator: non default invariant mass cut without updated subweights requested!")

    if finalstate == 'e+e-e+e-':
        pass
    elif finalstate == 'e+e-mu+mu-':
        aafh_mode = 3
        if not subweights:
            aafh_subgeneratorWeights = [1.000e+00, 1.520e+01, 3.106e+03, 6.374e+03, 1.000e+00, 1.778e+00, 6.075e+00, 6.512e+00]
        else:
            aafh_subgeneratorWeights = subweights
    else:
        B2FATAL("add_aafh_generator final state not supported: {}".format(finalstate))

    aafh = path.add_module(
        'AafhInput',
        mode=aafh_mode,
        rejection=2,
        maxSubgeneratorWeight=aafh_maxSubgeneratorWeight,
        maxFinalWeight=aafh_maxFinalWeight,
        subgeneratorWeights=aafh_subgeneratorWeights,
        suppressionLimits=[1e100] * 4,
        minMass=minmass
    )

    if preselection:
        generatorpreselection = path.add_module(
            'GeneratorPreselection',
            nChargedMin=1,
            MinChargedPt=0.1,
            MinChargedTheta=17.0,
            MaxChargedTheta=150.0
        )

        generator_emptypath = create_path()
        generatorpreselection.if_value('!=11', generator_emptypath)


def add_kkmc_generator(path, finalstate=''):
    """
    Add the default muon pair and tau pair generator KKMC

    Parameters:
        path (basf2.Path): path where the generator should be added
        finalstate(str): either "mu+mu-" or "tau+tau-"
    """

    #: kkmc input file
    kkmc_inputfile = Belle2.FileSystem.findFile('data/generators/kkmc/tau.input.dat')

    #: kkmc file that will hold cross section and other information
    kkmc_logfile = 'kkmc_tautau.txt'

    #: kkmc configuration file, should be fine as is
    kkmc_config = Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat')

    #: tau config file (empty for mu+mu-)
    kkmc_tauconfigfile = Belle2.FileSystem.findFile('data/generators/kkmc/tau_decaytable.dat')

    if finalstate == 'tau+tau-':
        pass
    elif finalstate == 'mu+mu-':
        kkmc_inputfile = Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat')
        kkmc_logfile = 'kkmc_mumu.txt'
        kkmc_tauconfigfile = ''
    else:
        B2FATAL("add_kkmc_generator final state not supported: {}".format(finalstate))

    # use KKMC to generate lepton pairs
    kkgeninput = path.add_module(
        'KKGenInput',
        tauinputFile=kkmc_inputfile,
        KKdefaultFile=kkmc_config,
        taudecaytableFile=kkmc_tauconfigfile,
        kkmcoutputfilename=kkmc_logfile,
    )


def add_evtgen_generator(path, finalstate=''):
    """
    Add EvtGen for mixed and charged BB

    Parameters:
        path (basf2.Path): path where the generator should be added
        finalstate (str): Either "charged" for B+/B- or "mixed" for B0/anti-B0
    """
    evtgen_userdecfile = Belle2.FileSystem.findFile('data/generators/evtgen/charged.dec')

    if finalstate == 'charged':
        pass
    elif finalstate == 'mixed':
        evtgen_userdecfile = Belle2.FileSystem.findFile('data/generators/evtgen/mixed.dec')
    else:
        B2FATAL("add_evtgen_generator final state not supported: {}".format(finalstate))

    # use EvtGen
    evtgen = path.add_module(
        'EvtGenInput',
        userDECFile=evtgen_userdecfile
    )


def add_continuum_generator(path, finalstate, userdecfile='', useevtgenparticledata=0, *, skip_on_failure=True):
    """
    Add the default continuum generators KKMC + PYTHIA including their default decfiles and PYTHIA settings

    See Also:
        `add_inclusive_continuum_generator()` to add continuum generation with preselected particles

    Parameters:
        path (basf2.Path): path where the generator should be added
        finalstate (str): uubar, ddbar, ssbar, ccbar
        userdecfile (str): EvtGen decfile used for particle decays
        useevtgenparticledata (bool): Experimental feature to use a consistent
            set of particle properties between EvtGen and PYTHIA
        skip_on_failure (bool): If True stop event processing right after
            fragmentation fails. Otherwise continue normally
    """

    #: kkmc input file, one for each qqbar mode
    kkmc_inputfile = Belle2.FileSystem.findFile('data/generators/kkmc/uubar_nohadronization.input.dat')

    #: kkmc file that will hold cross section and other information
    kkmc_logfile = 'kkmc_uubar.txt'

    #: pythia configuration, different for ccbar
    pythia_config = Belle2.FileSystem.findFile('data/generators/modules/fragmentation/pythia_belle2.dat')

    #: user decay file
    decay_user = Belle2.FileSystem.findFile('data/generators/modules/fragmentation/dec_belle2_qqbar.dec')
    if userdecfile == '':
        pass
    else:
        B2INFO('Replacing default user decfile: ', userdecfile)
        decay_user = userdecfile

    #: kkmc configuration file, should be fine as is
    kkmc_config = Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat')

    #: global decay file
    decay_file = get_default_decayfile()

    if finalstate == 'uubar':
        pass
    elif finalstate == 'ddbar':
        kkmc_inputfile = Belle2.FileSystem.findFile('data/generators/kkmc/ddbar_nohadronization.input.dat')
        kkmc_logfile = 'kkmc_ddbar.txt'
    elif finalstate == 'ssbar':
        kkmc_inputfile = Belle2.FileSystem.findFile('data/generators/kkmc/ssbar_nohadronization.input.dat')
        kkmc_logfile = 'kkmc_ssbar.txt'
    elif finalstate == 'ccbar':
        kkmc_inputfile = Belle2.FileSystem.findFile('data/generators/kkmc/ccbar_nohadronization.input.dat')
        pythia_config = Belle2.FileSystem.findFile('data/generators/modules/fragmentation/pythia_belle2_charm.dat')
        kkmc_logfile = 'kkmc_ccbar.txt'
    else:
        B2FATAL("add_continuum_generator final state not supported: {}".format(finalstate))

    # use KKMC to generate qqbar events (no fragmentation at this stage)
    kkgeninput = path.add_module(
        'KKGenInput',
        tauinputFile=kkmc_inputfile,
        KKdefaultFile=kkmc_config,
        taudecaytableFile='',
        kkmcoutputfilename=kkmc_logfile,
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
        useEvtGenParticleData=useevtgenparticledata
    )

    if skip_on_failure:
        # branch to an empty path if PYTHIA failed, this will change the number of events
        # but the file meta data will contain the total number of generated events
        generator_emptypath = create_path()
        fragmentation.if_value('<1', generator_emptypath)


def add_inclusive_continuum_generator(path, finalstate, particles, userdecfile='',
                                      useevtgenparticledata=0, *, include_conjugates=True, max_iterations=100000):
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
        finalstate (str): uubar, ddbar, ssbar, ccbar
        particles (list): A list of particle names or pdg codes. An event is
           only accepted if at lease one of those particles appears in the event.
        userdecfile (str): EvtGen decfile used for particle decays
        useevtgenparticledata (bool): Experimental feature to use a consistent
            set of particle properties between EvtGen and PYTHIA
        include_conjugates (bool): If True (default) accept the event also if a
            charge conjugate of the given particles is found
        max_iterations (int): maximum tries per event to generate the requested
            particle. If exceeded processing will be stopped with a
            `FATAL <LogLevel.FATAL>` error so for rare particles one might need a
            larger number.
    """
    loop_path = create_path()
    # we might run this more than once so make sure we remove any particles
    # before generating new ones
    loop_path.add_module("PruneDataStore", keepMatchedEntries=False, matchEntries=["MCParticles"])
    # add the generator but make sure it doesn't stop processing on
    # fragmentation failure as is this currently not supported by do_while
    add_continuum_generator(loop_path, finalstate, userdecfile, useevtgenparticledata, skip_on_failure=False)
    # check for the particles we want
    loop_path.add_module("InclusiveParticleChecker", particles=particles)
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
        B2FATAL("add_bhwide_generator minimum angle too small (<0.0) or too large (>180): {}".format(minangle))

    bhwide = path.add_module("BHWideInput")
    bhwide.param('ScatteringAngleRangePositron', [minangle, 180.0 - minangle])
    bhwide.param('ScatteringAngleRangeElectron', [minangle, 180.0 - minangle])
    bhwide.param('MaxAcollinearity', 180.0)
    bhwide.param('MinEnergy', 0.10)
    bhwide.param('VacuumPolarization', 'burkhardt')
    bhwide.param('WeakCorrections', True)
    bhwide.param('WtMax', 3.0)


def add_babayaganlo_generator(path, finalstate='', minenergy=0.15, minangle=10.0):
    """
    Add the high precision QED generator BABAYAGA.NLO to the path. Settings correspond to cross sections in BELLE2-NOTE-PH-2015-006

    Parameters:
        path (basf2.Path): path where the generator should be added
        finalstate (str): ee or gg
        minenergy (float): minimum particle energy in GeV
        minangle (float): angular range from minangle to 180-minangle for primary particles (in degrees)
    """

    babayaganlo = path.add_module("BabayagaNLOInput")

    if finalstate == 'ee':
        babayaganlo.param('FinalState', 'ee')
        babayaganlo.param('ScatteringAngleRange', [minangle, 180.0 - minangle])
        babayaganlo.param('MinEnergy', minenergy)
        babayaganlo.param('FMax', 1.e5)

    elif finalstate == 'gg':
        babayaganlo.param('FinalState', 'gg')
        babayaganlo.param('ScatteringAngleRange', [minangle, 180.0 - minangle])
        babayaganlo.param('MinEnergy', minenergy)
        babayaganlo.param('FMax', 1.e4)

    else:
        B2FATAL("add_babayaganlo_generator final state not supported: {}".format(finalstate))


def add_phokhara_generator(path, finalstate=''):
    """
    Add the high precision QED generator PHOKHARA to the path. Almost full
    acceptance settings for photons and hadrons/muons.

    Parameters:
        path (basf2.Path): path where the generator should be added
        finalstate (str): One of the possible final state "mu+mu-", "pi+pi-", "pi+pi-pi0"
    """

    phokhara = path.add_module('PhokharaInput')

    if finalstate == 'mu+mu-':
        phokhara.param('FinalState', 0)
        phokhara.param('LO', 0)  # force ISR production, no non-radiative production
        phokhara.param('NLO', 1)  # use full two loop corrections
        phokhara.param('QED', 0)  # use ISR only, no FSR, no interference

    elif finalstate == 'pi+pi-':
        phokhara.param('FinalState', 1)
        phokhara.param('LO', 0)  # force ISR production, no non-radiative production
        phokhara.param('NLO', 1)  # use full two loop corrections
        phokhara.param('QED', 0)  # use ISR only, no FSR, no interference

    elif finalstate == 'pi+pi-pi0':
        phokhara.param('FinalState', 8)
        phokhara.param('LO', 0)  # force ISR production, no non-radiative production
        phokhara.param('NLO', 0)  # no two loop corrections
        phokhara.param('QED', 0)  # use ISR only, no FSR, no interference
    else:
        B2FATAL("add_phokhara_generator final state not supported: {}".format(finalstate))


def add_cosmics_generator(path, components=None,
                          global_box_size=None, accept_box=None, keep_box=None,
                          geometry_xml_file='geometry/GCR_Summer2017.xml',
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
    import cdc.cr as cosmics_setup

    if global_box_size is None:
        global_box_size = [100, 100, 100]
    if accept_box is None:
        accept_box = [8, 8, 8]
    if keep_box is None:
        keep_box = [8, 8, 8]

    cosmics_setup.set_cdc_cr_parameters(data_taking_period)

    if cosmics_setup.cosmics_period == "201607":
        B2FATAL("The data taking period 201607 is very special (geometry setup, PMTs etc). This is not handled "
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
    cry.param('CosmicDataDir', Belle2.FileSystem.findFile(cosmics_data_dir))

    # user input file
    cry.param('SetupFile', Belle2.FileSystem.findFile(setup_file))

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
        cosmics_selector = register_module('CDCCosmicSelector',
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

        empty_path = create_path()
        cosmics_selector.if_false(empty_path)
