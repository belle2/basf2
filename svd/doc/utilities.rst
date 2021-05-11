.. _svdutilities:

SVD Utility Functions
=====================

.. autofunction:: svd.__init__.add_svd_reconstruction
		  :noindex:

.. autofunction:: svd.__init__.add_svd_SPcreation

.. autofunction:: svd.__init__.add_svd_create_recodigits
		  :noindex:
.. autofunction:: svd.__init__.add_rel5_svd_reconstruction

.. autofunction:: svd.__init__.add_svd_unpacker

.. autofunction:: svd.__init__.add_svd_unpacker_simulate3sampleDAQ

.. autofunction:: svd.__init__.add_svd_simulation
		  :noindex:
.. autofunction:: svd.__init__.add_svd_packer

event skim utils
----------------

To use these skim modules, first import it:

.. code::

   svd.skim_utils import skimOutRNDTrgModule

then use it:

.. code::

   skimRNDtrg = skimOutRNDTrgModule()
   main.add_module(skimRNDtrg)
   emptypath = create_path()  
   skimRNDtrg.if_false(emptypath)

.. autofunction:: svd.skim_utils.skimOutRNDTrgModule

.. autofunction:: svd.skim_utils.skimSVDBurstEventsModule

.. autofunction:: svd.skim_utils.skim6SampleEventsPyModule

.. autofunction:: svd.skim_utils.skimSVDTriggerBinEventsPyModule

.. autofunction:: svd.skim_utils.skimFineTRGEventsPyModule

.. autofunction:: svd.skim_utils.skimLowEventT0EventsPyModule

calibration validation utils
----------------------------

pleas check ``svd/scripts/svd/validation_utils.py``

background overlay utils
------------------------

.. note::

   These functions are not part of the official background overlay tools. Use them only if you know what you are doing!

.. autofunction:: svd.overlay_utils.prepare_svd_overlay

.. autofunction:: svd.overlay_utils.overlay_svd_data
