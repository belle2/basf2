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
        'patch_release-07_cdc_dedx_injection',
        'patch_main_release-07',
        'validation_2020-08-11',
        'online_2020-08-11',
        'Legacy_IP_Information']
    return globaltags
