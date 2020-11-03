import basf2 as b2
import b2test_utils as b2u
import ROOT


def filter_input_output(input_file_name, output_file_name, include_branches=None, exclude_branches=None):
    child_path = b2.Path()
    child_path.add_module('RootInput',
                          inputFileName=input_file_name)
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
    b2.process(child_path, 10)


def check_file_content(file_name, input_branches):
    file = ROOT.TFile.Open(file_name, 'READ')
    if file.IsZombie():
        b2.B2FATAL(f'Cannot open the input file {file_name}.')
    tree = file.Get('tree')
    file_branches = [branch.GetName() for branch in tree.GetListOfBranches()]
    file.Close()
    return set(input_branches) <= set(file_branches)


if __name__ == '__main__':
    with b2u.clean_working_directory():

        input_file_name = b2.find_file('framework/tests/mdst_with_muid.root')

        # Let's check if some branches are present.
        assert (
            1 == check_file_content(
                input_file_name, [
                    'Tracks', 'ECLClusters', 'TracksToECLClusters', 'Muids', 'TracksToMuids']))

        # Let's exclude Muids.
        file_to_check = 'mdst_without_muid.root'
        assert (0 == b2u.run_in_subprocess(input_file_name, file_to_check, None, ['Muids'],
                                           target=filter_input_output))
        assert (0 == check_file_content(file_to_check, ['Muids']))
        assert (0 == check_file_content(file_to_check, ['TrakcsToMuids']))
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
