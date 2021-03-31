import basf2 as b2
import b2test_utils as b2u
import ROOT


def filter_input_output(input_file_name, output_file_name, include_branches=None, exclude_branches=None):

    b2u.configure_logging_for_tests()
    b2.set_random_seed('Forza Livorno!!!')
    b2.B2INFO('We are testing the following configuration:',
              include_branches=include_branches,
              exclude_branches=exclude_branches)
    child_path = b2.Path()
    child_path.add_module('RootInput',
                          inputFileName=input_file_name,
                          collectStatistics=True)
    if include_branches is not None:
        b2.set_module_parameters(child_path,
                                 'RootInput',
                                 branchNames=include_branches)
    if exclude_branches is not None:
        b2.set_module_parameters(child_path,
                                 'RootInput',
                                 excludeBranchNames=exclude_branches)
    child_path.add_module('RootOutput',
                          outputFileName=output_file_name)
    b2.process(child_path)


def check_file_content(file_name, input_branches):

    f = ROOT.TFile.Open(file_name, 'READ')
    if f.IsZombie():
        b2.B2FATAL(f'Cannot open the input file {file_name}.')
    tree = f.Get('tree')
    file_branches = [branch.GetName() for branch in tree.GetListOfBranches()]
    f.Close()
    return set(input_branches) <= set(file_branches)


if __name__ == '__main__':

    with b2u.clean_working_directory():
        # Let's check the input file
        input_file_name = b2.find_file('framework/tests/mdst_with_muid.root')
        assert (0 == b2u.run_in_subprocess(input_file_name, 'mdst_all_branches.root', None, None,
                                           target=filter_input_output))
        all_branches = ['Tracks', 'ECLClusters', 'TracksToECLClusters', 'Muids', 'TracksToMuids']
        assert (1 == check_file_content(input_file_name, all_branches))
        # Let's exclude Muids.
        file_to_check = 'mdst_without_muid.root'
        assert (0 == b2u.run_in_subprocess(input_file_name, file_to_check, None, ['Muids'],
                                           target=filter_input_output))
        assert (0 == check_file_content(file_to_check, ['Muids']))
        assert (0 == check_file_content(file_to_check, ['TracksToMuids']))
        assert (1 == check_file_content(file_to_check, ['Tracks', 'ECLClusters']))
        # Let's include only Muids.
        file_to_check = 'mdst_only_muid.root'
        assert (0 == b2u.run_in_subprocess(input_file_name, file_to_check, ['Muids'], None,
                                           target=filter_input_output))
        # The dictionary is not found, so it can't be stored.
        assert (0 == check_file_content(file_to_check, ['Muids']))
        # Let's include only Tracks and ECLClusters.
        assert (0 == b2u.run_in_subprocess(input_file_name, file_to_check, ['Tracks', 'ECLClusters'], None,
                                           target=filter_input_output))
        assert (0 == check_file_content(file_to_check, ['Muids', 'TracksToMuids']))
        assert (1 == check_file_content(file_to_check, ['Tracks', 'ECLClusters', 'TracksToECLClusters']))
