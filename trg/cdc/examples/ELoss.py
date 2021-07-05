#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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
