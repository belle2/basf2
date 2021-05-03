.. _svddatastore:

SVD DataStore
=============

Here we briefly describe the SVD objects that populate the DataStore.

.. warning:
   this is not a detailed description, please check svd/dataobjects if you want to know more

``SVDEventInfo``, StoreObjPtr created by ``SVDUnpacker`` / ``SVDDigitizer`` and storing the following event-wise informations:

#. the DAQ mode (3-sample, 6-sample, 3-mixed-6 sample) of the run
#. the number of APV samples (3 or 6) in acquired the event
#. the trigger bin

``SVDShaperDigit``, StoreArray ``SVDUnpacker`` / ``SVDDigitizer`` and storing the raw strip informations:

#. layer, ladder, sensor (VxdID), side
#. cellID
#. amplitudes of the 6 samples, in case of 3-sample events, the last 3 values are set tot zero

``SVDRecoDigit``, StoreArray created by ``SVDRecoDigitCreator`` (not in default recosntruction) and storing the calibrated strip informations:

#. layer, ladder, sensor (VxdID), side
#. cellID
#. time and charge of the strip

``SVDCluster``, StoreArray created by ``SVDClusterizer`` and storing the 1-D hit informations:

#. layer, ladder, sensor (VxdID), side
#. size, i.e. number of strips in the cluster
#. position, time and charge of the cluster
   
