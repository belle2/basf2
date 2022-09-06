##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Wrappers around options and interfaces defined within the Belle2::MVA namespace.
'''


def CombinationOptions():
    '''
    Wrapper around CombinationOptions Belle2::MVA:CombinationOptions().
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.CombinationOptions()


def FANNOptions():
    '''
    Wrapper around FANNOptions Belle2::MVA:FANNOptions().
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.FANNOptions()


def FastBDTOptions():
    '''
    Wrapper around FastBDTOptions Belle2::MVA:FastBDTOptions().
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.FastBDTOptions()


def GeneralOptions():
    '''
    Wrapper around GeneralOptions Belle2::MVA:GeneralOptions().
    '''
    import ROOT  # noqa
    return ROOT.Belle2.MVA.GeneralOptions()


def MetaOptions():
    '''
    Wrapper around MetaOptions Belle2::MVA:MetaOptions().
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.MetaOptions()


def PDFOptions():
    '''
    Wrapper around PDFOptions Belle2::MVA:PDFOptions().
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.PDFOptions()


def PythonOptions():
    '''
    Wrapper around PythonOptions Belle2::MVA:PythonOptions().
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.PythonOptions()


def RegressionFastBDTOptions():
    '''
    Wrapper around RegressionFastBDTOptions Belle2::MVA:RegressionFastBDTOptions().
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.RegressionFastBDTOptions()


def ReweighterOptions():
    '''
    Wrapper around ReweighterOptions Belle2::MVA:ReweighterOptions().
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.ReweighterOptions()


def TMVAOptionsClassification():
    '''
    Wrapper around TMVAOptionsClassification Belle2::MVA:TMVAOptionsClassification().
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.TMVAOptionsClassification()


def TMVAOptionsMulticlass():
    '''
    Wrapper around TMVAOptionsMulticlass Belle2::MVA:TMVAOptionsMulticlass().
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.TMVAOptionsMulticlass()


def TMVAOptionsRegression():
    '''
    Wrapper around TMVAOptionsRegression Belle2::MVA:TMVAOptionsRegression().
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.TMVAOptionsRegression()


def TrivialOptions():
    '''
    Wrapper around TrivialOptions Belle2::MVA:TrivialOptions().
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.TrivialOptions()
