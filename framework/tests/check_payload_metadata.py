'''
Check the revision number of a payload used by a module.
'''

import os

import basf2 as b2
import b2test_utils as b2tu
import ROOT.Belle2 as B2


class CheckPayloadMetadata(b2.Module):
    '''
    Small module for checking some payload metadata.
    '''

    def __init__(self, use_testing_payloads):
        '''Consructor.'''
        super(CheckPayloadMetadata, self).__init__()
        #: BeamParameters database object.
        self.beam_parameters = B2.PyDBObj('BeamParameters')
        #: Flag to mark if testing payloads are used or not.
        self.use_testing_payloads = use_testing_payloads

    def beginRun(self):
        '''Begin run.'''
        assert(self.beam_parameters.isValid())
        if self.use_testing_payloads:
            assert(self.beam_parameters.getRevision() == 0)
            assert(os.path.basename(self.beam_parameters.getFilename()).startswith('dbstore'))
            assert(self.beam_parameters.getIoV() == B2.IntervalOfValidity(0, 1, 2, 3))
        else:
            assert(self.beam_parameters.getRevision() > 0)
            assert(self.beam_parameters.getFilename().startswith('/cvmfs'))
            assert(self.beam_parameters.getIoV() == B2.IntervalOfValidity(0, 0, 0, -1))


def create_beam_parameters_payload(testing_database_path):
    '''
    Create locally a BeamParameters payload.
    '''
    b2.conditions.expert_settings(save_payloads=testing_database_path)
    beam_parameters = B2.BeamParameters()
    database = B2.Database.Instance()
    iov = B2.IntervalOfValidity(0, 1, 2, 3)
    assert(database.storeData('BeamParameters', beam_parameters, iov))


def testing_path(use_testing_payloads=False, testing_database_path=None, remove_database=False):
    '''
    The basf2 steering path for this test.
    '''
    b2.conditions.reset()
    if remove_database:
        b2.conditions.override_globaltags()
    elif use_testing_payloads:
        b2.conditions.prepend_testing_payloads(b2.find_file(testing_database_path))
    testing_path = b2.Path()
    testing_path.add_module('EventInfoSetter', expList=[0], runList=[1])
    testing_path.add_module(CheckPayloadMetadata(use_testing_payloads=use_testing_payloads))
    return testing_path


if __name__ == '__main__':

    with b2tu.clean_working_directory():

        # Path to the dabase with the testing payloads.
        testing_database_path = 'testingdb/testingdb.txt'

        # Create the testing payloads.
        assert(b2tu.run_in_subprocess(target=create_beam_parameters_payload, testing_database_path=testing_database_path) == 0)

        # Check if the process aborts because no valid payloads are found.
        assert(b2tu.safe_process(testing_path(remove_database=True)) > 0)

        # Check if the default globaltag is correctly used.
        assert(b2tu.safe_process(testing_path(use_testing_payloads=False)) == 0)

        # Check if the testing payload is correctly used.
        assert(b2tu.safe_process(testing_path(use_testing_payloads=True, testing_database_path=testing_database_path)) == 0)
