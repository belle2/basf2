#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from ROOT import Belle2


# Dictionary containing global tag names for Belle II upgrade flavors with VTX
valid_upgrade_globaltags = {"upgrade_globaltag_vtx_7layer": "upgrade_2020-09-22_vtx_7layer",
                            "upgrade_globaltag_vtx_5layer": "upgrade_2020-09-22_vtx_5layer",
                            "upgrade_globaltag_vtx_5layer_discs": "upgrade_2020-07-13_vtx_5layer_discs",
                            "upgrade_globaltag_vtx_tb_2020-03": "upgrade_2020-07-07_VTX-CMOS-testbeam-2020-03-12",
                            }


def add_vtx_reconstruction(path, clusterName=None, digitsName=None, useVTXClusterShapes=True):

    if useVTXClusterShapes:
        if 'ActivateVTXClusterPositionEstimator' not in [e.name() for e in path.modules()]:
            path.add_module('ActivateVTXClusterPositionEstimator').set_name('ActivateVTXClusterPositionEstimator')

    if 'VTXClusterizer' not in [e.name() for e in path.modules()]:
        if clusterName is None:
            clusterName = ''
        if digitsName is None:
            digitsName = ''
        path.add_module('VTXClusterizer', Clusters=clusterName, Digits=digitsName)

    # Add VTXSpacePointCreator
    add_vtx_SPcreation(path, clusterName=clusterName)


def add_vtx_SPcreation(path, clusterName=None):

    vtxSPCreatorName = 'VTXSpacePointCreator'
    vtx_clusters = ''
    if clusterName:
        vtx_clusters = clusterName
    nameSPs = 'VTXSpacePoints'

    if vtxSPCreatorName not in [e.name() for e in path.modules()]:
        path.add_module('VTXSpacePointCreator', NameOfInstance='VTXSpacePoints',
                        SpacePoints=nameSPs, VTXClusters=vtx_clusters).set_name(vtxSPCreatorName)


def add_vtx_simulation(path, digitsName=None):

    if digitsName is None:
        digitsName = ''

    path.add_module('VTXDigitizer', Digits=digitsName)


def get_upgrade_globaltag():
    """ Returns the name of the upgrade global tag from the environmant variable
    BELLE2_VTX_UPGRADE_GT or raises an error if the variable is not set.
    """

    import os
    from basf2 import B2ERROR, B2INFO

    env_name = 'BELLE2_VTX_UPGRADE_GT'
    upgrade_globaltag = ''

    if env_name not in os.environ:
        valid_gts = ", ".join(valid_upgrade_globaltags.values())
        raise RuntimeError(
            "Environment variable {:s} not set. Pick a globaltag from {:s}. Terminanting this script.".format(
                env_name, valid_gts))
    upgrade_globaltag = os.environ[env_name]

    B2INFO("Using upgrade globaltag {}".format(upgrade_globaltag))

    return upgrade_globaltag


def get_upgrade_background_files(folder=None, output_file_info=True):
    """ Loads the location of the background files from the environmant variable
    BELLE2_VTX_BACKGROUND_DIR and ensures that background
    files exist and returns the list of background files which
    can be directly used with add_simulation() :

    >>> add_simulation(main, bkgfiles=vtx.get_upgrade_background_files())

    Will fail with an assert if no background folder set or if no background file was
    found in the set folder.

    Parameters:
        folder (str): A specific folder to search for background files can be given as an optional parameter
        output_file_info (str): If true, a list of the found background files and there size will be printed
                                This is useful to understand later which background campaign has been used
                                to simulate events.
    """

    import os
    import glob
    from basf2.utils import pretty_print_table
    from basf2 import create_path, B2ERROR, B2INFO

    env_name = 'BELLE2_VTX_BACKGROUND_DIR'
    bg = None

    if folder is None:
        if env_name not in os.environ:
            raise RuntimeError("Environment variable {} for backgound files not set. Terminanting this script.".format(env_name))
        folder = os.environ[env_name]

    # Special value to run w/o bg files
    if folder == 'NONE' or folder == 'None':
        B2INFO("Background files set to None. No overlay will be done")
        return None

    upgrade_gt_name = get_upgrade_globaltag()
    if upgrade_gt_name not in folder:
        raise RuntimeError("Upgrade GT {} not contained in path to bg files. Terminanting this script.".format(upgrade_gt_name))

    bg = glob.glob(folder + '/*.root')

    if len(bg) == 0:
        raise RuntimeError("No background files found in folder {} . Terminating this script.".format(folder))

    B2INFO("Background files loaded from folder {}".format(folder))

    # sort for easier comparison
    bg = sorted(bg)

    if output_file_info:
        bg_sizes = [os.path.getsize(f) for f in bg]
        # reformat to work with pretty_print_table
        table_rows = [list(entry) for entry in zip(bg, bg_sizes)]
        table_rows.insert(0, ["- Background file name -", "- file size -"])

        pretty_print_table(table_rows, [0, 0])

    return bg


def get_upgrade_background_mixer_files(folder=None, output_file_info=True):
    """ Loads the location of the background mixer files from the environmant variable
    BELLE2_VTX_BACKGROUND_MIXING_DIR and ensures that background
    files exist and returns the list of background files which
    can be directly used with add_simulation() :

    >>> add_simulation(main, bkgfiles=vtx.get_upgrade_background_mixer_files())

    Will fail with an assert if no background folder set or if no background file was
    found in the set folder.

    Parameters:
        folder (str): A specific folder to search for background files can be given as an optional parameter
        output_file_info (str): If true, a list of the found background files and there size will be printed
                                This is useful to understand later which background campaign has been used
                                to simulate events.
    """

    import os
    import glob
    from basf2.utils import pretty_print_table
    from basf2 import create_path, B2ERROR, B2INFO

    env_name = 'BELLE2_VTX_BACKGROUND_MIXING_DIR'
    bg = None

    if folder is None:
        if env_name not in os.environ:
            raise RuntimeError("Environment variable {} for backgound files not set. Terminanting this script.".format(env_name))
        folder = os.environ[env_name]

    bg = glob.glob(folder + '/*.root')

    if len(bg) == 0:
        raise RuntimeError("No background files found in folder {} . Terminating this script.".format(folder))

    B2INFO("Background mixer files loaded from folder {}".format(folder))

    # sort for easier comparison
    bg = sorted(bg)

    if output_file_info:
        bg_sizes = [os.path.getsize(f) for f in bg]
        # reformat to work with pretty_print_table
        table_rows = [list(entry) for entry in zip(bg, bg_sizes)]
        table_rows.insert(0, ["- Background file name -", "- file size -"])

        pretty_print_table(table_rows, [0, 0])

    return bg
