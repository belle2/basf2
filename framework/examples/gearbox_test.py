#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

bk = gearbox.create_backend('Postgres1', {'connectionString'
                            : 'host=b2db01.cc.kek.jp dbname=belle2 user=belle2 password=belle2db port=80'
                            , 'tag': 'head'})

bk2 = gearbox.create_backend('Xml', {'filename'
                             : 'framework/examples/gearbox_testdata.xml'})

gearbox.mount_backend(bk, '/', '/', EMountMode.overlay)

gearbox.mount_backend(bk2, '/rootnode', '/belle2/Detector/SuperKEKB',
                      EMountMode.merge)

gearbox.testQuery('/belle2/Detector/SuperKEKB//*')

# gearbox.testQuery("/belle2/Detector/SuperKEKB//*|//SupportRibs//*")

