'''
File summarizing all default generator settings.
More information: BELLE2-NOTE-PH-2015-006
Contact: Torben Ferber (ferber@physics.ubc.ca)
'''

from basf2 import *
from ROOT import Belle2
import os


def get_default_decayfile():
    """Return the default DECAY.dec for Belle2"""
    return Belle2.FileSystem.findFile("generators/evtgen/decayfiles/DECAY_BELLE2.DEC")


def add_aafh_generator(path, finalstate='', preselection=False):
    """
    Add the default two photon generator for four fermion final states
    :param finalstate: e+e-e+e-, e+e-mu+mu-
    :param preselection: if true, select events with at least one medium pt particle in the CDC acceptance
    """

    aafh = register_module('AafhInput')
    aafh_mode = 5
    aafh_subgeneratorWeights = [1.0, 7.986e+01, 5.798e+04, 3.898e+05, 1.0, 1.664e+00, 2.812e+00, 7.321e-01]
    aafh_maxSubgeneratorWeight = 1.0
    aafh_maxFinalWeight = 3.0

    if finalstate == 'e+e-e+e-':
        pass
    elif finalstate == 'e+e-mu+mu-':
        aafh_mode = 3
        aafh_subgeneratorWeights = [1.000e+00, 1.520e+01, 3.106e+03, 6.374e+03, 1.000e+00, 1.778e+00, 6.075e+00, 6.512e+00]
        aafh_maxSubgeneratorWeight = 1.0
    else:
        B2FATAL("add_aafh_generator final state not supported: ", finalstate)

    aafh = path.add_module(
        'AafhInput',
        mode=aafh_mode,
        rejection=2,
        maxSubgeneratorWeight=aafh_maxSubgeneratorWeight,
        maxFinalWeight=aafh_maxFinalWeight,
        subgeneratorWeights=aafh_subgeneratorWeights,
        suppressionLimits=[1e100] * 4,
        minMass=0.50
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
    :param finalstate: mu+mu-, tau+tau-
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
        B2FATAL("add_kkmc_generator final state not supported: ", finalstate)

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
    """
    evtgen_userdecfile = Belle2.FileSystem.findFile('data/generators/evtgen/charged.dec')

    if finalstate == 'charged':
        pass
    elif finalstate == 'mixed':
        evtgen_userdecfile = Belle2.FileSystem.findFile('data/generators/evtgen/mixed.dec')
    else:
        B2FATAL("add_evtgen_generator final state not supported: " + str(finalstate))

    # use EvtGen
    evtgen = path.add_module(
        'EvtGenInput',
        userDECFile=evtgen_userdecfile
    )


def add_continuum_generator(path, finalstate='', userdecfile=''):
    """
    Add the default continuum generators KKMC + PYTHIA including their default decfiles and PYTHIA settings
    :param finalstate: uubar, ddbar, ssbar, ccbar
    :param emptypathname branch to reject events where PYTHIA failed to fragment
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

    #: global decay file, should be fine as is
    decay_file = os.path.expandvars('$BELLE2_EXTERNALS_DIR/share/evtgen/DECAY_2010.DEC')

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
        B2FATAL("add_continuum_generator final state not supported: ", finalstate)

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
    )

    # branch to an empty path if PYTHIA failed, this will change the number of events
    # but the file meta data will contain the total number of generated events
    generator_emptypath = create_path()
    fragmentation.if_value('<1', generator_emptypath)


def add_babayaganlo_generator(path, finalstate=''):
    """
    Add the high precision QED generator BABAYAGA.NLO to the path. Settings correspond to cross sections in BELLE2-NOTE-PH-2015-006
    :param path: Add the modules to this path
    :param finalstate: ee or gg
    """

    babayaganlo = path.add_module("BabayagaNLOInput")

    if finalstate == 'ee':
        babayaganlo.param('FinalState', 'ee')
        babayaganlo.param('ScatteringAngleRange', [10.0, 170.0])
        babayaganlo.param('MinEnergy', 0.15)
        babayaganlo.param('FMax', 1.e5)

    elif finalstate == 'gg':
        babayaganlo.param('FinalState', 'gg')
        babayaganlo.param('ScatteringAngleRange', [10.0, 170.0])
        babayaganlo.param('MinEnergy', 0.15)
        babayaganlo.param('FMax', 1.e4)

    else:
        B2FATAL("add_babayaganlo_generator final state not supported: ", finalstate)


def add_phokhara_generator(path, finalstate=''):
    """
    Add the high precision QED generator PHOKHARA to the path. Almost full acceptance settings for photons and hadrons/muons.
    :param path: Add the modules to this path
    :param finalstate: One of the possible final states using the PHOKHARA particle naming
    """

    phokhara = path.add_module('')

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
        B2FATAL("add_phokhara_generator final state not supported: ", finalstate)


def add_cosmics_generator(path, components=None, global_box_size=None, accept_box=None,
                          keep_box=None,
                          geometry_xml_file='geometry/GCR_Summer2017.xml',
                          cosmics_data_dir='data/generators/modules/cryinput/',
                          setup_file='simulation/scripts/cry.setup'):
    """
    Add the cosmics generator CRY with the default parameters to the path.
    :param path: Add the modules to this path.
    :param components: list of geometry components to add in the geometry module,
           or None for all components.
    :param global_box_size: sets global length, width and height.
    :param accept_box: sets the size of the accept box. As a default it is
           set to 8.0 m as Belle2 detector size.
    :param keep_box: sets the size of the keep box (keep box >= accept box).
    :param geometry_xml_file: Name of the xml file to use for the geometry.
    :param cosmics_data_dir: parameter CosmicDataDir for the cry module (absolute or relative to the basf2 repo).
    :param setup_file: location of the cry.setup file (absolute or relative to the basf2 repo)
    """
    if global_box_size is None:
        global_box_size = [20, 20, 9]
    if accept_box is None:
        accept_box = [8, 8, 8]
    if keep_box is None:
        keep_box = [8, 8, 8]

    if 'Gearbox' not in path:
        path.add_module('Gearbox', override=[
            ("/Global/length", str(global_box_size[0]), "m"),
            ("/Global/width", str(global_box_size[1]), "m"),
            ("/Global/height", str(global_box_size[2]), "m")],
            fileName=geometry_xml_file,
        )

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
