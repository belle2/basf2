# !/usr/bin/env python
# -*- coding: utf-8 -*-

##
# contributors: G. Casarosa, J. Bennett
##

######
# util to skim out the event triggered with random trigger
#
# in order to use this function:
# import it:
#     from svd.skim_utils import skimOutRNDTrgModule
# and then use it:
#     skimRNDtrg = skimOutRNDTrgModule()
#     main.add_module(skimRNDtrg)
#     emptypath = create_path()
#     skimRNDtrg .if_false(emptypath)
####

import basf2
import ROOT
from ROOT import Belle2


class skimOutRNDTrgModule(basf2.Module):

    def event(self):

        rawFTSW = Belle2.PyStoreArray('RawFTSWs')

        if not rawFTSW.isValid():
            B2ERROR('No RawFTSW available - event ignored')
            self.return_value(0)

            return

        # unknown meaning of this number
        unknownInt = 0
        if rawFTSW[0].GetTRGType(unknownInt) != Belle2.TRGSummary.TTYP_RAND:
            self.return_value(1)
        else:
            self.return_value(0)
