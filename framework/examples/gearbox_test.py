#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

bk = gearbox.create_backend('Postgres1',
                            {'connectionString': 'host=belledb.ijs.si dbname=belle2 user=belle2 password=belle2db port=5432'
                            , 'schema': 'public', 'tag': 'head'})

bk2 = gearbox.create_backend('Xml',
                             {'filename': 'framework/examples/gearbox_testdata.xml'
                             })

bk3 = gearbox.create_backend('Postgres2', {'schema': 'eklm',
                             'meta_table': 'config_t',
                             'connectionString': 'host=belledb.ijs.si dbname=belle2 user=belle2 password=belle2db port=5432'
                             })

gearbox.mount_backend(bk, '/', '/', EMountMode.overlay)

gearbox.mount_backend(bk2, '/rootnode', '/belle2/Detector/SuperKEKB',
                      EMountMode.merge)

gearbox.mount_backend(bk3, '/', '/belle2/detectors/eklm', EMountMode.merge)

gearbox.testQuery('/belle2/detectors/eklm/geometry[@id< 10]')

# gearbox.testQuery('/belle2/Detector/SuperKEKB//*')

# gearbox.testQuery("/belle2/Detector/SuperKEKB//*|//SupportRibs//*")

