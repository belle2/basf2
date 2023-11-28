.. _KLMCaibration:

This seems the best place to cite our internal note `KLM offline calibration`_.

.. _KLM offline calibration: https://docs.belle2.org/record/1848?ln=en

Creation of default payloads
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The default payloads are created using scripts in the directory
``examples/database``.

.. list-table::
   :widths: 40 60
   :header-rows: 1

   * - Script
     - Payloads
   * - ``ImportAlignment.py``
     - ``BKLMAlignment``, ``EKLMAlignment``, ``EKLMSegmentAlignment``
   * - ``ImportChannelStatus.py``
     - ``KLMChannelStatus``
   * - ``ImportElectronicsMap.py``
     - ``KLMElectronicsMap``
   * - ``ImportGeometry.py``
     - ``BKLMGeometryPar``, ``EKLMGeometry``
   * - ``ImportLikelihoodParameters.py``
     - ``KLMLikelihoodParameters``
   * - ``ImportReconstructionParameters.py``
     - ``KLMReconstructionParameters``
   * - ``ImportScintillatorDigitizationParameters.py``
     - ``KLMScintillatorDigitizationParameters``
   * - ``ImportScintillatorFEEParameters.py``
     - ``KLMScintillatorFEEParameters``
   * - ``ImportStripEfficiency.py``
     - ``KLMStripEfficiency``
   * - ``ImportTimeCableDelay.py``
     - ``KLMTimeCableDelay``
   * - ``ImportTimeConstants.py``
     - ``KLMTimeConstants``
   * - ``ImportTimeConversion.py``
     - ``KLMTimeConversion``
   * - ``ImportTimeResolution.py``
     - ``KLMTimeResolution``
   * - ``ImportTimeWindow.py``
     - ``ImportTimeWindow``

Note that the script ``ImportGeometry.py`` creates a testing geometry
configuration (payload ``GeoConfiguration``) containing KLM only.
The complete geometry can be created by
the script ``geometry/examples/create_geometry_payloads.py``.

Channel status
~~~~~~~~~~~~~~

Alignment
~~~~~~~~~

Strip efficiency
~~~~~~~~~~~~~~~~

Time
~~~~
