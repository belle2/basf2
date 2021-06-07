#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

        from variables import variables as vm
        from ROOT import Belle2

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

        from ROOT import Belle2

        super().__init__()
        self.skim = skim
        self.EventExtraInfo = Belle2.PyStoreObj("EventExtraInfo")

    def initialize(self):
        """
        Check EventExtraInfo has been registered previously. This registration should be
        done by InitialiseSkimFlag.
        """
        self.EventExtraInfo.isRequired()

    def event(self):
        """
        Check if at least one skim list is non-empty; if so, update the skim flag to 1.
        """

        from ROOT import Belle2

        ListObjects = [Belle2.PyStoreObj(lst) for lst in self.skim.SkimLists]

        # Check required skim lists have been built on this path
        if any([not ListObj.isValid() for ListObj in ListObjects]):
            b2.B2FATAL(
                f"Error in UpdateSkimFlag for {self.skim}: particle lists not built. "
                "Did you add this module to the pre-skim path rather than the post-skim path?"
            )

        nCandidates = sum(ListObj.getListSize() for ListObj in ListObjects)

        # Override ExtraInfo flag if at least one candidate from any list passed
        if nCandidates > 0:
            self.EventExtraInfo.setExtraInfo(self.skim.flag, 1)
