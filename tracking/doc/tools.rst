.. _tracking_tools:

Tools
-----

.. warning::
  This documentation is under construction!


Tools for SectorMap training
""""""""""""""""""""""""""""
The tracking tools contain four scripts related to the SectorMap training. A detailed description on how a full SectorMap training is performed can be found in :ref:`tracking_secMapTraining`

* :command:`tracking-vxdtf2-collect-train-data` : collects data needed for training a SectorMap by simulating MC events
* :command:`tracking-vxdtf2-prepare-SectorMap` : Once a SectorMap is built it has to be prepared. This preparation includes generating a version of the 
  SectorMap with cuts on SVD timing removed, and the generation of the payloads and data base files needed for upload to the DB. 
* :command:`tracking-vxdtf2-submitAllCollectionJobs` : In its default setting this script submits all jobs to a queue to create a full trainings dataset for the SectorMap training. Note: it assumes that a working LSF queue is attached (e.g. at KEKcc). Internally the tracking-vxdtf2-collect-train-data is called. If no LSF queue is attached either use the :command:`tracking-vxdtf2-collect-train-data` script or adapt the :command:`tracking-vxdtf2-submitAllCollectionJobs` script. 
* :command:`tracking-vxdtf2-train-SectorMap` : Performs the actual training of the SectorMap using an input sample created by the :command:`tracking-vxdtf3-collect-train-data` or :command:`tracking-vxdtf2-submitAllCollectionJobs` scripts. 

These tools can be accessed anywhere once `basf2` is set up. To get a full list of available options run the corresponding script with the `--help` option.  
