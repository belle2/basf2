#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Modules required for calculating skim flags. Skim flags track whether an event passes a
skim, without the need to directly remove those events from processing.
"""

import basf2 as b2


class InitialiseSkimFlag(b2.Module):
    """
    *[Module for skim expert usage]* Create the EventExtraInfo DataStore object, and set
    all required flag variables to zero.

    .. Note::

        Add this module to the path before adding any skims, so that the skim flags are
        defined in the datastore for all events.
    """

    def __init__(self, *skims):
        """
        Initialise module.

        Parameters:
            skims (skim.core.BaseSkim): Skim to initialise event flag for.
        """

        from ROOT import Belle2  # noqa
        from variables import variables as vm  # noqa

        super().__init__()
        self.skims = skims
        self.EventExtraInfo = Belle2.PyStoreObj("EventExtraInfo")

        # Create aliases for convenience
        for skim in skims:
            vm.addAlias(skim.flag, f"eventExtraInfo({skim.flag})")

    def initialize(self):
        """
        Register EventExtraInfo in datastore if it has not been registered already.
        """
        if not self.EventExtraInfo.isValid():
            self.EventExtraInfo.registerInDataStore()

    def event(self):
        """
        Initialise flags to zero.
        """
        self.EventExtraInfo.create()
        for skim in self.skims:
            self.EventExtraInfo.addExtraInfo(skim.flag, 0)


class UpdateSkimFlag(b2.Module):
    """
    *[Module for skim expert usage]* Update the skim flag to be 1 if there is at least
    one candidate in any of the skim lists.

    .. Note::

        Add this module to the post-skim path of each skim in the combined skim, as the
        skim lists are only guaranteed to exist on the conditional path (if a
        conditional path was used).
    """

    def __init__(self, skim):
        """
        Initialise module.

        Parameters:
            skim (skim.core.BaseSkim): Skim to update event flag for.
        """

        from ROOT import Belle2  # noqa

        super().__init__()
        self.skim = skim
        self.EventExtraInfo = Belle2.PyStoreObj("EventExtraInfo")
        self.ListObjects = []

    def initialize(self):
        """
        Check EventExtraInfo and all the necessary ParticleLists have been registered previously.
        The registration of EventExtraInfo should be done by InitialiseSkimFlag.
        """
        from ROOT import Belle2  # noqa

        self.EventExtraInfo.isRequired()
        self.ListObjects = [Belle2.PyStoreObj(lst) for lst in self.skim.SkimLists]
        for ListObject in self.ListObjects:
            ListObject.isRequired()

    def event(self):
        """
        Check if at least one skim list is non-empty; if so, update the skim flag to 1.
        """
        flag = 0
        for ListObject in self.ListObjects:
            if not ListObject.isValid():
                b2.B2FATAL(
                    f"Error in UpdateSkimFlag for {self.skim}: particle lists not built. "
                    "Did you add this module to the pre-skim path rather than the post-skim path?"
                )
            elif ListObject.getListSize() > 0:
                flag = 1

        self.EventExtraInfo.setExtraInfo(self.skim.flag, flag)
