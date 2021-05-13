.. _svddatastore:

SVD DataStore
=============

Here we briefly describe the SVD objects that populate the DataStore.

.. warning:
   this is not a detailed description, please check svd/dataobjects if you want to know more

.. _svdeventinfo:

.. cpp:class:: SVDEventInfo

	       StoreObjPtr created by :b2:mod:`SVDUnpacker` / :b2:mod:`SVDEventInfoSetter` and storing the following event-wise informations:

	       #. the DAQ mode (3-sample, 6-sample, 3-mixed-6 sample) of the run
	       #. the number of APV samples (3 or 6) in the acquired event
	       #. the :ref:`TriggerBin<svdtb>`

.. _svdshapers:

.. cpp:class:: SVDShaperDigit

	       StoreArray created by :b2:mod:`SVDUnpacker` / :b2:mod:`SVDDigitizer` and storing the raw strip informations:

	       #. layer, ladder, sensor (VxdID), side
	       #. cellID
	       #. amplitudes of the 6 samples. In case of 3-sample events, the last 3 values are set to zero

.. _svdrecos:

.. cpp:class:: SVDRecoDigit

	       StoreArray created by :b2:mod:`SVDRecoDigitCreator` (not in default reconstruction) and storing the calibrated strip informations:

	       #. layer, ladder, sensor (VxdID), side
	       #. cellID
	       #. time and charge of the strip

.. _svdclusters:

.. cpp:class:: SVDCluster

	       StoreArray created by :b2:mod:`SVDClusterizer` and storing the 1-D hit informations:

	       #. layer, ladder, sensor (VxdID), side
	       #. size, i.e. number of strips in the cluster
	       #. position, time and charge of the cluster
   

.. _svdsps:

.. cpp:class:: SVDSpacePoint

	       StoreArray created by the :b2:mod:`SVDSpacePointCreator` and storing the 3-d  hit informations
