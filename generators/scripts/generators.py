'''
File for summarizing all default generator settings.
'''
from ROOT import Belle2


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
