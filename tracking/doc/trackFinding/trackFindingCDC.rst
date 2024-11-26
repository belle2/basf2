.. _tracking_trackFindingCDC:

CDC Track Finding
^^^^^^^^^^^^^^^^^

.. warning::
  This documentation is under construction!

In this section we briefly describe the CDC standalone track finding.

CDC Quality Indicator
"""""""""""""""""""""
Finally, all CDCTracks with a CDC quality indicator below threshold are rejected.
The CDC Quality Indicator is the output of a BDT that was designed and trained to reject fake and clone tracks while maintaining a high finding efficiency.
More information is given in the :ref:`Track quality indicators<tracking_qualityIndicators>` section.