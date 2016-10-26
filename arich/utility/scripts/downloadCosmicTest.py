#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os

com = "git archive --remote=ssh://git@stash.desy.de:7999/b2g/arich.git HEAD database/cosmicTest_payloads | tar -x"
os.system(com)

com = "git archive --remote=ssh://git@stash.desy.de:7999/b2g/arich.git HEAD datafiles/cosmicTest | tar -x"
os.system(com)

com = "gunzip -r datafiles/cosmicTest"
os.system(com)

com = "mv datafiles ${BELLE2_LOCAL_DIR}"
os.system(com)

com = "mv database ${BELLE2_LOCAL_DIR}/arich/"
os.system(com)
