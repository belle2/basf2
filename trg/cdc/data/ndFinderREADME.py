#!/usr/bin/env python3

###################################
# 3D hit data
###################################
#
# A 1/32 phi-sector of the CDC contains 41 axial TS and 32 stereo TS,
# each with 3 priority positions.
# The full track parameter space is binned with (omega, phi, theta) = (40, 384, 9).
# A 1/32 phi-sector contains 12 bins in phi.
#
# The theta axis is only required for stereo TS.
#
# In the zero suppressed version, the phi-axis has 15 bins:
#   phi_start, phi_width, phi_0, ..., phi_12
#   with    0 < phi_start < 132
#           0 < phi_width < 13
#
# The maximum allowed phi-extent of a hit in track space
# is set to 11/32 phi-sectors corresponding to 11 * 12 = 132 bins.
#
# The non-zero suppressed hit curves in the track parameter space
# can be reconstructed by adding phi_start as an offset to the
# fields (phi_0, ..., phi_12). phi_width is the number of
# non-zero elements in (phi_0, ..., phi_12).

###################################
# Example: reading the 3D hit data
###################################

import numpy as np

# Files containing the zero suppressed track to hit relations
# (hit curves in track parameter space).
fileAxial = 'ndFinderAxialShallow.txt.gz'
fileStereo = 'ndFinderStereoShallow.txt.gz'

axialFlat = np.loadtxt(fileAxial, dtype='uint8')
stereoFlat = np.loadtxt(fileStereo, dtype='uint8')
axialComp = axialFlat.reshape((41, 3, 40, 15))
stereoComp = stereoFlat.reshape((32, 3, 40, 15, 9))
