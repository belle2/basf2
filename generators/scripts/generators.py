'''
File for summarizing all default generator settings.
'''


def add_cosmics_generator(path, accept_box=None, keep_box=None,
                          cosmics_data_dir='data/generators/modules/cryinput/',
                          setup_file='simulation/scripts/cry.setup'):
    """
    Add the cosmics generator CRY with the default parameters to the path.
    """
    if accept_box is None:
        accept_box = [8, 8, 8]
    if keep_box is None:
        keep_box = [8, 8, 8]

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
