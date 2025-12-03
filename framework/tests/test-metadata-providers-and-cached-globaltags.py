##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test that basf2 runs smoothly if we use globaltags that are not in a local database.
Test also that the order of the providers does not matter in case we pass non-existing globaltags,
or globaltags with a non-allowed status (e.g. OPEN).
'''

import basf2 as b2
import b2test_utils as b2tu


def path_for_test(globaltags, providers):
    b2.conditions.metadata_providers = providers
    for globaltag in globaltags:
        b2.conditions.append_globaltag(globaltag)
    b2.set_random_seed('114')
    main = b2.Path()
    main.add_module('EventInfoSetter')
    main.add_module('PrintBeamParameters')
    b2.process(main)


if __name__ == '__main__':

    if b2tu.is_cdb_down():
        b2tu.skip_test('Test currently disabled due to CDB troubles')

    b2tu.configure_logging_for_tests(replace_cdb_provider=False)

    local = '/cvmfs/belle.cern.ch/conditions/database.sqlite'
    central = 'http://belle2db.sdcc.bnl.gov/b2s/rest/'

    for providers in [[central], [central, local], [local, central]]:
        # This must pass, no matter what
        assert (b2tu.run_in_subprocess(target=path_for_test, globaltags=[], providers=providers) == 0)
        # We append a PUBLISHED globaltag, not cached -> must pass
        assert (b2tu.run_in_subprocess(target=path_for_test, globaltags=['user_depietro_cdb_test_1'], providers=providers) == 0)
        # We append a OPEN globaltag, not cached -> must fail
        assert (b2tu.run_in_subprocess(target=path_for_test, globaltags=['user_depietro_cdb_test_2'], providers=providers) != 0)
        # We append a INVALID globaltag, not cached -> must fail
        assert (b2tu.run_in_subprocess(target=path_for_test, globaltags=['user_depietro_cdb_test_3'], providers=providers) != 0)
        # We append a not existing globaltag -> must fail
        assert (b2tu.run_in_subprocess(target=path_for_test, globaltags=['114'], providers=providers) != 0)
        # We append also a OPEN globaltag, not cached -> must fail
        assert (
            b2tu.run_in_subprocess(
                target=path_for_test,
                globaltags=[
                    'user_depietro_cdb_test_1',
                    'user_depietro_cdb_test_2'],
                providers=providers) != 0)
        # We append also a INVALID globaltag, not cached -> must fail
        assert (
            b2tu.run_in_subprocess(
                target=path_for_test,
                globaltags=[
                    'user_depietro_cdb_test_1',
                    'user_depietro_cdb_test_3'],
                providers=providers) != 0)
        # We append a OPEN and a INVALID globaltag, not cached -> must fail
        assert (
            b2tu.run_in_subprocess(
                target=path_for_test,
                globaltags=[
                    'user_depietro_cdb_test_2',
                    'user_depietro_cdb_test_3'],
                providers=providers) != 0)
        # We append also a not existing globaltag -> must fail
        assert (
            b2tu.run_in_subprocess(
                target=path_for_test,
                globaltags=[
                    'user_depietro_cdb_test_1',
                    '114'],
                providers=providers) != 0)
        # We append also a not existing globaltag -> must fail
        assert (
            b2tu.run_in_subprocess(
                target=path_for_test,
                globaltags=[
                    'user_depietro_cdb_test_2',
                    '114'],
                providers=providers) != 0)
        # We append also a not existing globaltag -> must fail
        assert (
            b2tu.run_in_subprocess(
                target=path_for_test,
                globaltags=[
                    'user_depietro_cdb_test_1',
                    'user_depietro_cdb_test_2',
                    '114'],
                providers=providers) != 0)

    for providers in [[local]]:
        # This must pass, no matter what
        assert (b2tu.run_in_subprocess(target=path_for_test, globaltags=[], providers=providers) == 0)
        # We append a not-cached globaltag -> must fail
        assert (b2tu.run_in_subprocess(target=path_for_test, globaltags=['user_depietro_cdb_test_1'], providers=providers) != 0)
        # We don't test the other cases, they are all equivalent to the previous one
