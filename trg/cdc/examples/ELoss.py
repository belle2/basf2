#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# --------------------------------------------------------------
# Filename : ELoss.py
# Owner    : KyungTae Kim (K.U.)
# Email    : ktkim@hep.korea.ac.kr
# --------------------------------------------------------------
# Description : Patch to turn off the physics process in Geant4
#               assuming cdc package is added in local
# --------------------------------------------------------------

import os

os.system("patch -p0 < ELoss.patch")
os.system("patch -p0 < CDC_SCons.patch")
