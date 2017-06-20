'''
File for summarizing all default generator settings.
Contact: Torben Ferber (ferber@physics.ubc.ca)
'''

from basf2 import *
from ROOT import Belle2
import os


def add_kkmc_generator(path, finalstate='tau+tau-'):
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

    if finalstate == 'mu+mu-':
        kkmc_inputfile = Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat')
        kkmc_logfile = 'kkmc_mumu.txt'
        kkmc_tauconfigfile = ''

    # use KKMC to generate lepton pairs
    kkgeninput = path.add_module(
        'KKGenInput',
        tauinputFile=kkmc_inputfile,
        KKdefaultFile=kkmc_config,
        taudecaytableFile=kkmc_tauconfigfile,
        kkmcoutputfilename=kkmc_logfile,
    )


def add_continuum_generator(path, finalstate='uubar', emptypathname='generator_emptypath'):
    """
    Add the default continuum generators KKMC + PYTHIA including their default decfiles and OPYTHIA settings
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

    #: kkmc configuration file, should be fine as is
    kkmc_config = Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat')

    #: global decay file, should be fine as is
    decay_file = os.path.expandvars('$BELLE2_EXTERNALS_DIR/share/evtgen/DECAY_2010.DEC')

    if finalstate == 'ddbar':
        kkmc_inputfile = Belle2.FileSystem.findFile('data/generators/kkmc/ddbar_nohadronization.input.dat')
        kkmc_logfile = 'kkmc_ddbar.txt'
    elif finalstate == 'ssbar':
        kkmc_inputfile = Belle2.FileSystem.findFile('data/generators/kkmc/ssbar_nohadronization.input.dat')
        kkmc_logfile = 'kkmc_ssbar.txt'
    elif finalstate == 'ccbar':
        kkmc_inputfile = Belle2.FileSystem.findFile('data/generators/kkmc/ccbar_nohadronization.input.dat')
        pythia_config = Belle2.FileSystem.findFile('data/generators/modules/fragmentation/pythia_belle2_charm.dat')
        kkmc_logfile = 'kkmc_ccbar.txt'

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
    emptypath = create_path()
    fragmentation.if_value('<1', emptypath)


def add_babayaganlo_generator(path, finalstate='ee'):
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


def add_phokhara_generator(path, finalstate='mu+mu-'):
    """
    Add the high precision QED generator PHOKHARA to the path. Almost full acceptance settings for photons and hadrons/muons.
    :param path: Add the modules to this path
    :param finalstate: One of the possible final states using the PHOKHARA particle naming
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


def add_cosmics_generator(path, components=None, global_box_size=None, accept_box=None,
                          keep_box=None,
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
            ("/Global/height", str(global_box_size[2]), "m")]
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
