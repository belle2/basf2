##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test for checking if b2conditionsdb-tag-merge behaves as expted:
- highest priority to the first tag in the list;
- within the same tag, highest priority to the payloads with the highest revision number.
'''


import subprocess

import conditions_db as cdb


if __name__ == '__main__':

    tags = ['main_tag_merge_test_1', 'main_tag_merge_test_2', 'main_tag_merge_test_3']
    final_tag = 'main_tag_merge_test_0'

    db = cdb.ConditionsDB()

    for tag in tags:
        payloads = db.get_all_iovs(globalTag=tag)
        payloads.sort(key=lambda p: p.iov)

        for payload in payloads:
            print(f'  {payload.name} r{payload.revision} {payload.iov}')

    subprocess.check_call(['b2conditionsdb-tag-merge', '-o', final_tag, '--dry-run'] + tags)
