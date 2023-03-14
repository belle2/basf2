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

.. autofunction:: svd.skim_utils.skimLowHighEventT0EventsPyModule

example:

.. code::

   # select events with |EventT0| < maxAbsEvtT0 and |EventT0| > minAbsEvtT0
   maxAbsEvtT0 = 25 # in ns
   minAbsEvtT0 = -1 # in ns
   skim = b2.register_module(skimLowHighEventT0EventsPyModule())
   skim.set_maxAbsEventT0( maxAbsEvtT0 )
   skim.set_minAbsEventT0( minAbsEvtT0 )
   main.add_module(skim)
   emptypath = b2.create_path()
   skim.if_false(emptypath)

calibration validation utils
----------------------------

pleas check ``svd/scripts/svd/validation_utils.py``

background overlay utils
------------------------

.. warning::

   These functions are not part of the official background overlay tools. Use them only if you know what you are doing!

.. autofunction:: svd.overlay_utils.prepare_svd_overlay

.. autofunction:: svd.overlay_utils.overlay_svd_data
