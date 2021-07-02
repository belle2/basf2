#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import math
from ROOT import Belle2

if __name__ == "__main__":
    # parameters for the field
    b_field = (0, 0, 0)
    range_r = (0, math.inf)
    range_z = (-math.inf, math.inf)

    # and for the payload
    iov = Belle2.IntervalOfValidity.always()
    # iov = Belle2.IntervalOfValidity(5,0,-1,-1)

    # create field map instance
    field = Belle2.MagneticField()
    # and if we have a non-zero field, add it as component
    if max(abs(e) for e in b_field) > 0:
        field_vector = Belle2.B2Vector3D(*(e * Belle2.Unit.T for e in b_field))
        field.addComponent(Belle2.MagneticFieldComponentConstant(field_vector, *range_r, *range_z))

    # and save in localdb/, ready for upload
    Belle2.Database.Instance().storeData("MagneticField", field, iov)
