##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Wrappers around functions defined as static methods of the Belle2::MVA::Utility class.
'''


def available(filename, experiment=0, run=0, event=0):
    '''
    Wrapper around bool Belle2::MVA::Utility::available(
      const std::string& filename, int experiment = 0, int run = 0, int event = 0)
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.Utility.available(filename, experiment, run, event)


def download(identifier, filename, experiment=0, run=0, event=0):
    '''
    Wrapper around Belle2::MVA::Utility::download(
      const std::string& identifier, const std::string& filename, int experiment = 0, int run = 0, int event = 0)
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    ROOT.Belle2.MVA.Utility.download(identifier, filename, experiment, run, event)


def expert(*args):
    '''
    Wrapper around Belle2::MVA::Utility::expert(
      const std::vector<std::string>& filenames, const std::vector<std::string>& datafiles,
      const std::string& treename, const std::string& outputfile, int experiment = 0, int run = 0, int event = 0,
      bool copy_target = true);
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    ROOT.Belle2.MVA.Utility.expert(*args)


def extract(filename, directory):
    '''
    Wrapper around Belle2::MVA::Utility::extract(const std::string& filename, const std::string& directory)
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    ROOT.Belle2.MVA.Utility.extract(filename, directory)


def info(filename):
    '''
    Wrapper around std::string Belle2::MVA::Utility::info(const std::string& filename)
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.Utility.info(filename)


def save_custom_weightfile(*args):
    '''
    Wrapper around Belle2::MVA::Utility::save_custom_weightfile(
      const GeneralOptions& general_options, const SpecificOptions& specific_options,
      const std::string& custom_weightfile, const std::string& output_identifier = "")
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    ROOT.Belle2.MVA.Utility.save_custom_weightfile(*args)


def teacher(*args):
    '''
    Wrapper around Belle2::MVA::Utility::teacher(
      const GeneralOptions& general_options, const SpecificOptions& specific_options,
      const MetaOptions& meta_options = MetaOptions())
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    ROOT.Belle2.MVA.Utility.teacher(*args)


def teacher_dataset(*args):
    '''
    Wrapper around std::unique_ptr<Belle2::MVA::Expert> Belle2::MVA::Utility::teacher_dataset(
      GeneralOptions general_options, const SpecificOptions& specific_options, Dataset& data)
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.Utility.teacher_dataset(*args)


def teacher_sideband_subtraction(*args):
    '''
    Wrapper around std::unique_ptr<Belle2::MVA::Expert> Belle2::MVA::Utility::teacher_sideband_subtraction(
      GeneralOptions general_options, const SpecificOptions& specific_options, const MetaOptions& meta_options)
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.Utility.teacher_sideband_subtraction(*args)


def teacher_splot(*args):
    '''
    Wrapper around std::unique_ptr<Belle2::MVA::Expert> Belle2::MVA::Utility::teacher_splot(
      GeneralOptions general_options, const SpecificOptions& specific_options, const MetaOptions& meta_options)
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.Utility.teacher_splot(*args)


def teacher_reweighting(*args):
    '''
    Wrapper around std::unique_ptr<Belle2::MVA::Expert> Belle2::MVA::Utility::teacher_reweighting(
      GeneralOptions general_options, const SpecificOptions& specific_options, const MetaOptions& meta_options)
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    return ROOT.Belle2.MVA.Utility.teacher_reweighting(*args)


def upload(filename, identifier, exp1=0, run1=0, exp2=-1, run2=-1):
    '''
    Wrapper around Belle2::MVA::Utility::upload(
      const std::string& filename, const std::string& identifier, int exp1 = 0, int run1 = 0, int exp2 = -1,
      int run2 = -1)
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    ROOT.Belle2.MVA.Utility.upload(filename, identifier, exp1, run1, exp2, run2)


def upload_array(*args):
    '''
    Wrapper around Belle2::MVA::Utility::upload_array(
      const std::vector<std::string>& filenames, const std::string& identifier, int exp1 = 0, int run1 = 0,
      int exp2 = -1, int run2 = -1)
    '''
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    ROOT.Belle2.MVA.Utility.upload_array(*args)
