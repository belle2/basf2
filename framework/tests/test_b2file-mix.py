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

import subprocess

import basf2 as b2
import b2test_utils as b2tu


class PopulateEvents(b2.Module):
    '''Class for populating the events with some random numbers.'''

    def initialize(self):
        '''Initialize.'''
        import ROOT
        #: EventMetaData
        self.evt = ROOT.Belle2.PyStoreObj('EventMetaData')
        self.evt.isRequired()

    def event(self):
        '''Event.'''
        import ROOT
        # Let's store a random number in the slot for the generator weight
        self.evt.setGeneratedWeight(ROOT.gRandom.Rndm())


def path_for_test_file(events, output_name, seed):
    b2.set_random_seed(seed)
    main = b2.Path()
    main.add_module('EventInfoSetter', expList=0, runList=0, evtNumList=events)
    main.add_module(PopulateEvents())
    main.add_module('RootOutput', outputFileName=output_name)
    b2.process(main)


if __name__ == '__main__':

    with b2tu.clean_working_directory():

        events = 1000
        file_name_a = 'test_a.root'
        file_name_b = 'test_b.root'
        file_name_ab = 'test_ab.root'

        # Create a test file in the current directory
        b2tu.run_in_subprocess(target=path_for_test_file, events=events, output_name=file_name_a, seed='aaa')
        b2tu.run_in_subprocess(target=path_for_test_file, events=events, output_name=file_name_b, seed='bbb')
        subprocess.check_call(['b2file-merge', file_name_ab, file_name_a, file_name_b])

        # Now mix the file and run a simple check
        subprocess.check_call(['b2file-mix', file_name_a, '-o', 'test1.root'])
        metadata = b2.get_file_metadata('test1.root')
        assert (metadata.getNEvents() == events)

        # Mix again the file, but keeping less events
        subprocess.check_call(['b2file-mix', file_name_a, '-o', 'test2.root', '-n', '100'])
        metadata = b2.get_file_metadata('test2.root')
        assert (metadata.getNEvents() == 100)

        # Mix again the file, but changing the exp. number
        subprocess.check_call(['b2file-mix', file_name_a, '-o', 'test3.root', '--exp', '114'])
        metadata = b2.get_file_metadata('test3.root')
        assert (metadata.getExperimentLow() == 114)

        # Mix two files, this time passing the seed. Few times: twice with the same seed and once with a different one.
        subprocess.check_call(['b2file-mix', file_name_a, file_name_b, '-o', 'test4.root', '--seed', 'abc'])
        metadata = b2.get_file_metadata('test4.root')
        assert (metadata.getRandomSeed() == 'abc')
        subprocess.check_call(['b2file-mix', file_name_a, file_name_b, '-o', 'test5.root', '--seed', 'abc'])
        subprocess.check_call(['b2file-mix', file_name_a, file_name_b, '-o', 'test6.root', '--seed', 'def'])

        # Compare the content of test4, test5 and test6 and make sure it's different from test_ab.
        # Then, test4 and test5 must be identical, while test6 must be different than test4 and test5.
        # Since b2file-mix keep the relative ordering between the events in a file: For checking test6
        # is different, let's count how many times the entries differ and ask this number is large enough.
        # Because of the same reason, let's skip the first and last few events.
        import ROOT  # noqa
        counts = 0
        with ROOT.TFile.Open(file_name_ab) as fileab:
            treeab = fileab.Get('tree')
            with ROOT.TFile.Open('test4.root') as file4:
                tree4 = file4.Get('tree')
                with ROOT.TFile.Open('test5.root') as file5:
                    tree5 = file5.Get('tree')
                    with ROOT.TFile.Open('test6.root') as file6:
                        tree6 = file6.Get('tree')
                        for i, (eventab, event4, event5, event6) in enumerate(zip(treeab, tree4, tree5, tree6)):
                            if i < 10 or i > 1990:
                                continue
                            weightab = eventab.EventMetaData.getGeneratedWeight()
                            weight4 = event4.EventMetaData.getGeneratedWeight()
                            weight5 = event5.EventMetaData.getGeneratedWeight()
                            weight6 = event6.EventMetaData.getGeneratedWeight()
                            assert (weightab != weight4)
                            assert (weightab != weight6)
                            assert (weight4 == weight5)
                            counts += (weight4 != weight6)
        # It should be above 1900: let's be generous here.
        assert (counts > 1800)
