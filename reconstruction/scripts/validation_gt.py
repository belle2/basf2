##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


def get_validation_globaltags():
    # Legacy_IP_Information GT is for exp3 test file
    globaltags = [
        'temp_arich_geometry_geant4_11_1',
        'patch_main_release-07',
        'validation_2020-08-11',
        'online_2020-08-11',
        'Legacy_IP_Information']
    return globaltags
