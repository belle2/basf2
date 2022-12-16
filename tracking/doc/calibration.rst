.. _tracking_calibration: 

Tracking Calibrations
---------------------

Some tracking algorithms use MVAs that need to be calibrated, usually with the help of simulated events. 

.. _tracking_calibration_flipNrefit:

Flip & Refit MVA Training
^^^^^^^^^^^^^^^^^^^^^^^^^

Below the instructions how to retrain, apply and evaluate the MVAs for the :ref:`trk_flipNrefit` tool.

.. note::

  The Flip&Refit tool uses two MVAs:

  1. 1st MVA: select the RecoTracks to be flipped looking at low level variables (eg: ndf, cdc hits number, pxd hits etc)
  2. 2nd MVA: takes the fitResults fro the original and the flipped Tracks as input to decide wich one to keep

  The steps for sample generation, training and performance evlauation are similare, and are described in the following.

step 1: sample generation
"""""""""""""""""""""""""
A sample of 100k :math:`B\bar{B}` events is used for training.
The sample is generated using the script ``tracking/examples/BBbar_FlipAndRefitTest.py``

.. code:: bash

  basf2 BBbar_FlipAndRefitTest.py  -- --num {1,2} -n 1000 --exp 1003 --ranseed 23 --output_file_mva training_input.root 

.. tip::

  * the parameter ``--num`` indicates that the sample is generated for the training of the 1st MVA (with ``--num 1``) or the 2nd MVA (with ``--num 2``),
  * the rest of the parameters can be selected by the user
  * the option ``--flip_recoTrack True`` must be added when generating the samples for the second MVA
  * to speed up this step, the user can submit several jobs in parallel, paying attention to set different random seed and output file names for each job using the options ``--ranseed`` and ``--output_file_mva`` respectively.


step 2: MVA training
""""""""""""""""""""

In this step, a local database containing the payloads with the weightfiles of the first or second MVA will be created.
The payloads are created with the script ``tracking/flipAndRefitMVAs/training_flipping_mvaa.py``.
An example for the 1st MVA is reported below:

.. code:: bash

  basf2 tracking/flipAndRefitMVAs/training_flipping_mvas.py -train train_1stmva.root -data test_1stmva.root -tree data -mva 1

.. note::

  * the training and test samples can be generated using the instructions of the step 1
  * for additional informations on the parameters of the script, check its help:

    .. code:: bash

      basf2 tracking/flipAndRefitMVAs/training_flipping_mvas.py --h


step 3: 1st-MVA performance evaluation
""""""""""""""""""""""""""""""""""""""

The performance evaluation is done with the dedicated ``basf2`` tool for MVA evaluation, ``basf2_mva_evaluate.py``.
An example is reported in the following:

.. code:: bash

  basf2_mva_evaluate.py -id localdb/dbstore_Weightfile_rev_0b43a0.root -tree data -train train.root -data test.root -o validation.root

For further instructions, use the help of the executable.

.. code:: bash

  basf2_mva_evaluate.py --help


How to use the local weight file in the reconstruction
""""""""""""""""""""""""""""""""""""""""""""""""""""""

In case the user wants to use the local database, i.e. the payload created locally instead of the default one,
the user should modify the parameter ``identifier`` of the ``FlipQuality`` module.
Running the standard tracking chain, the easier thing to do is to directly mody the 
paremeter in the the utility function ``add_flipping_of_recoTracks`` in ``tracking/scripts/tracking/path_utils.py``
in the following way:

.. code:: python

  path.add_module("FlipQuality", recoTracksStoreArrayName=reco_tracks,
                  identifier='localdb/dbstore_Weightfile_rev_0b43a0.root',  
                  indexOfFlippingMVA=1).set_name("FlipQuality_1stMVA")

.. note:: 

  if new variables added in the MVA, make sure the ``/tracking/trackFitting/trackQualityEstimator/variableExtractors/include/FlipRecoTrackExtractor.h`` is also updated.



.. _tracking_calibration_CKF:

CKF Training
^^^^^^^^^^^^

.. warning ::
  work in progress