##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Wrappers around interfaces defined within the Belle2::MVA namespace.
'''


def Weightfile():
    '''
    Wrapper around Weightfile Belle2::MVA:Weightfile().
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.Weightfile()
