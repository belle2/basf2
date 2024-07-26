##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test the tool b2file-mix and make sure it works.
'''

import contextlib
import subprocess

import basf2 as b2
import b2test_utils as b2tu


def path_for_test_file(events, output_name):
    main = b2.Path()
    main.add_module('EventInfoSetter', expList=0, runList=0, evtNumList=events)
    main.add_module('RootOutput', outputFileName=output_name)
    b2.process(main)


@contextlib.contextmanager
def open_root(filename, mode='READ'):
    # Context manager for handling root files
    import ROOT
    f = ROOT.TFile.Open(filename, mode)
    try:
        yield f
    finally:
        if f:
            f.Close()


if __name__ == '__main__':

    with b2tu.clean_working_directory():

        events = 1000
        file_name = 'test.root'

        # Create a test file in the current directory
        b2tu.run_in_subprocess(target=path_for_test_file, events=events, output_name=file_name)

        # Now mix the file and run a simple check
        subprocess.check_call(['b2file-mix', file_name, '-o', 'test1.root'])
        metadata = b2.get_file_metadata('test1.root')
        assert (metadata.getNEvents() == events)

        # Mix again the file, but keeping less events
        subprocess.check_call(['b2file-mix', file_name, '-o', 'test2.root', '-n', '100'])
        metadata = b2.get_file_metadata('test2.root')
        assert (metadata.getNEvents() == 100)

        # Mix again the file, but changing the exp. number
        subprocess.check_call(['b2file-mix', file_name, '-o', 'test3.root', '--exp', '114'])
        metadata = b2.get_file_metadata('test3.root')
        assert (metadata.getExperimentLow() == 114)

        # Mix again the file, this time passing the seed. Twice, so we check the files are identical
        subprocess.check_call(['b2file-mix', file_name, '-o', 'test4.root', '--seed', 'a_seed'])
        metadata = b2.get_file_metadata('test4.root')
        assert (metadata.getRandomSeed() == 'a_seed')
        subprocess.check_call(['b2file-mix', file_name, '-o', 'test5.root', '--seed', 'a_seed'])

        # Compare the content of test4 and test5
        with open_root('test4.root') as file4:
            tree4 = file4.Get('tree')
            with open_root('test5.root') as file5:
                tree5 = file5.Get('tree')
                for event4, event5 in zip(tree4, tree5):
                    assert (event4.EventMetaData.getEvent() == event5.EventMetaData.getEvent())
