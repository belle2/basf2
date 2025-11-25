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

Electronics map
~~~~~~~~~~~~~~~

The electronics map (``KLMElectronicsMap`` payload) defines the mapping between
detector channels (physical strips in the KLM detector) and electronics channels
(readout channels in the data acquisition system). This mapping is essential for
unpacking raw data and associating readout information with the correct detector
elements.

Class ``KLMElectronicsMapImporter``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The class (defined in ``calibration/include/KLMElectronicsMapImporter.h``) provides 
the interface for creating and importing electronics map payloads into the conditions database.

**Basic usage:**

.. code-block:: python

   from ROOT.Belle2 import KLMElectronicsMapImporter
   
   # Create an importer instance
   importer = KLMElectronicsMapImporter()
   
   # Load default electronics maps
   importer.loadBKLMElectronicsMap(version)
   importer.loadEKLMElectronicsMap(version, mc)
   
   # Set the interval of validity (IOV)
   importer.setIOV(exp_low, run_low, exp_high, run_high)
   
   # Import to database
   importer.importElectronicsMap()

**Key methods:**

- ``loadBKLMElectronicsMap(int version)``: Load BKLM electronics map.
  
  - Version 1: Used before experiment 10
  - Version 2: Used from experiment 10 onwards (chimney sector mapping changed)

- ``loadEKLMElectronicsMap(int version, bool mc)``: Load EKLM electronics map.
  
  - Version 1: Phase 2, experiment 3 (wrong cable connections in backward sectors 2 and 3)
  - Version 2: Phase 3, starting from experiment 4
  - The ``mc`` flag indicates whether to load MC or data map (data maps may require additional cable switch corrections)

- ``setLane(...)``: Override the default lane mapping for specific detector elements.
  This is crucial for correcting physical fiber connection differences between the
  intended design and actual installation. Two overloaded versions are available:
  
  - Module-wide: ``setLane(subdetector, section, sector, layer, lane)``
  - Plane-wide: ``setLane(subdetector, section, sector, layer, plane, lane)``

- ``clearElectronicsMap()``: Clear the current electronics map (useful when loading
  multiple map versions sequentially with different IOVs)

- ``setIOV(exp_low, run_low, exp_high, run_high)``: Set the interval of validity
  for the payload. Use ``-1`` for unlimited upper bound.

- ``importElectronicsMap()``: Import the current electronics map to the database

**Example: Creating maps for different periods**

The script ``examples/database/ImportElectronicsMap.py`` demonstrates how to create
electronics maps for different data-taking periods. Key points:

- Different map versions are used for different experimental periods due to hardware
  changes (cable reconnections, repairs, etc.)
- MC maps are simpler and typically use a single version
- Data maps require corrections using ``setLane()`` to account for actual fiber
  connections that differ from the standard mapping

For example, experiment 37 run 975 onwards required lane corrections for EKLM
backward section 3 due to fiber swaps between layer 7 and layer 10.

.. code-block:: python

   # Example from ImportElectronicsMap.py
   load_bklm_electronics_map(2, False)
   load_eklm_electronics_map(2, False)
   # Correct for fiber swaps in EB2
   importer.setLane(KLMElementNumbers.c_EKLM,
                    EKLMElementNumbers.c_BackwardSection,
                    3, 10, 6)
   importer.setLane(KLMElementNumbers.c_EKLM,
                    EKLMElementNumbers.c_BackwardSection,
                    3, 7, 3)
   importer.setIOV(37, 975, -1, -1)
   importer.importElectronicsMap()

Important note on PCIe40 connections
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. warning::

   Before LS1, KLM readout used the COPPER system. After LS1,
   the upgrade to PCIe40 boards provides higher-density
   readout. However, the KLM unpacker still uses the COPPER data format internally,
   requiring a PCIe40-to-COPPER conversion step.
   
   The PCIe40 channel-to-COPPER mapping in the unpacker
   (``klm/modules/KLMUnpacker/src/KLMUnpackerModule.cc``, lines 300-313) is
   **hardcoded** and directly derives COPPER/HSLB IDs from PCIe40 channel numbers.
   
   **This means PCIe40 fibers must be connected in the exact order expected by
   the unpacker.** If connections are changed anywhere upstream to PCIe40, the entire mapping breaks.
   
   When fiber connections change, you must:
   
   1. Update the electronics map (frontend → DC mapping) using ``setLane()``
   2. **AND** either:
      
      a. Ensure PCIe40 channels maintain their expected order, OR
      b. Modify the unpacker code to reflect the new PCIe40 channel order
   
   Simply updating the electronics map with ``setLane()`` is **not sufficient**
   by itself, because the unpacker's hardcoded PCIe40-to-COPPER conversion will
   still produce incorrect channel assignments.

Channel status
~~~~~~~~~~~~~~

Alignment
~~~~~~~~~

Strip efficiency
~~~~~~~~~~~~~~~~

Time
~~~~
