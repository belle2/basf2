#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os

com = "git archive --remote=ssh://${BELLE2_USER}@stash.desy.de:7999/b2g/arich_database.git HEAD cosmicTest_payloads | tar -x"
os.system(com)

com = "git archive --remote=ssh://${BELLE2_USER}@stash.desy.de:7999/b2g/arich_datafiles.git HEAD cosmicTest | tar -x"
os.system(com)

com = "gunzip -r cosmicTest"
os.system(com)

com = "mkdir ${BELLE2_LOCAL_DIR}/datafiles"
os.system(com)
com = "mv cosmicTest ${BELLE2_LOCAL_DIR}/datafiles/"
os.system(com)

com = "mkdir ${BELLE2_LOCAL_DIR}/arich/database"
os.system(com)
com = "mv cosmicTest_payloads ${BELLE2_LOCAL_DIR}/arich/database/"
os.system(com)
