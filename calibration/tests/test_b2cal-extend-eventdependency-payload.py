import glob
import os
import subprocess
import basf2
import b2test_utils

'''
Test the b2cal-extend-eventdependency-tool.
'''

__author__ = 'Giacomo De Pietro'
__copyright__ = '2021 - Belle II Collaboration'


if __name__ == '__main__':

    with b2test_utils.clean_working_directory():

        import ROOT  # noqa

        # Location of the local database to be used during the whole test
        database_location = 'testingdb/testingdb.txt'
        basf2.conditions.expert_settings(save_payloads=database_location)

        # Create a simple EventDependency payload with a BeamSpot object there
        beamspot = ROOT.Belle2.BeamSpot()
        event_object = ROOT.Belle2.EventDependency(beamspot)
        database = ROOT.Belle2.Database.Instance()
        iov = ROOT.Belle2.IntervalOfValidity.always()
        created = database.storeData('BeamSpot', event_object, iov)
        if not created:
            basf2.B2FATAL('Something went wrong when creating the payload for the test')

        # Locate the payload file
        payload_file = glob.glob(f'{basf2.find_file(os.path.dirname(database_location))}/*root')[0]

        # Test if the tool exits with a proper return code in case of incorrect calls
        # First test: incorrect call, wrong payload file
        result = subprocess.run(['b2cal-extend-eventdependency-payload', '-i', 'foo.bar',
                                 '-n', 'BeamSpot', '--iov', '1', '2', '3', '4', '--output_db',
                                 f'{database_location}'])
        assert(not result.returncode == 0)
        # Second test: incorrect call, wrong payload name
        result = subprocess.run(['b2cal-extend-eventdependency-payload', '-i', f'{payload_file}',
                                 '-n', 'FooBar', '--iov', '1', '2', '3', '4', '--output_db',
                                 f'{database_location}'])
        assert(not result.returncode == 0)
        # Third test: incorrect call, incomplete IoV
        result = subprocess.run(['b2cal-extend-eventdependency-payload', '-i', f'{payload_file}',
                                 '-n', 'BeamSpot', '--iov', '1', '2', '3', '--output_db',
                                 f'{database_location}'])
        assert(not result.returncode == 0)
        # Fourth test: incorrect call, wrong database file
        result = subprocess.run(['b2cal-extend-eventdependency-payload', '-i', f'{payload_file}',
                                 '-n', 'BeamSpot', '--iov', '1', '2', '3', '4', '--output_db',
                                 'foo/bar'])
        assert(not result.returncode == 0)

        # Finally, test if the tool works fine if called correctly
        result = subprocess.run(['b2cal-extend-eventdependency-payload', '-i', f'{payload_file}',
                                 '-n', 'BeamSpot', '--iov', '1', '2', '3', '4', '--output_db',
                                 f'{database_location}'])
        assert(result.returncode == 0)

        # Check if the tool appended correctly the new payload to the existing database
        with open(database_location) as f:
            line_number = 0
            revision = 0
            for line in f:
                fields = line.split(' ')
                assert(fields[0] == 'dbstore/BeamSpot')
                # In the first line we have the first created payload: it must have an infinite IoV
                if line_number == 0:
                    assert(fields[2].strip() == '0,0,-1,-1')
                # While in the second line the IoV must be (1,2,3,4)
                elif line_number == 1:
                    assert(fields[2].strip() == '1,2,3,4')
                line_number += 1
