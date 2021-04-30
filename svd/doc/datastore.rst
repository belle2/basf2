.. _svddatastore:

SVD DataStore
==================

Here we briefly describe the SVD objects that populate the DataStore.

``SVDEventInfo``, StoreObjPtr storing the following event-wise informations:

#. the DAQ mode (3-sample, 6-sample, 3-mixed-6 sample) of the run
#. the number of APV samples (3 or 6) in acquired the event
#. the trigger bin

``SVDShaperDigit``, StoreArray storing the raw strip informations

#. layer,ladder,sensor (VxdID), side
#. cellID
#. amplitudes of the 6 samples, in case of 3-sample events, the last 3 values are set tot zero

``SVDRecoDigit``, StoreArray storing the calibrated strip informations:

#. layer,ladder,sensor (VxdID), side
#. cellID
#. time and charge of the strip

``SVDCluster``, StoreArray storing the 1-D hit informations:

#. layer,ladder,sensor (VxdID), side
#. position, time and charge of the cluster
#. cluster size, i.e. number of strips in the cluster
   
