Smart Background Simulation
---------------------------------------------------------------------

The Smart Background project aims to reduce the production time and resources required for directly 
skimmed background MC campaigns. To this end, a transformer based neural network is used to 
predict the probability that an event will pass a given skim directly after event generation 
before the costly simulation and reconstruction steps (see the figure TODO). To ensure unbiased distributions 
after filtering, importance sampling is employed, using the neural network output as a probability to sample 
an event and, if it is kept, weighting it with the inverse neural network output. 

.. note:: Datasets produced using Smart Background are weighted and must be treated as such when analyzed! The weights 
  are stored as generator weights and are therefore automatically written to all produced ntuples.

To employ this method, we recommend using the :py:func:`generators.add_smartbkg_filtering` convenience function 
from the generators package. It should be placed after the event generator but before simulation and reconstruction. 
As mandatory inputs it requires the LFN code of the skim you are running (available for all skims via skim.code), 
as well as the type of MC you are producing (uubar, ddbar, ssbar, ccbar, charged, mixed, taupair). A part of your 
steering file might then look like this (for a full example see ``generators/examples/SmartBkg_NEW_ExampleSteering.py``):

.. code-block:: python

  # Add event generator (evtgen for charged events in this example)
  finalstate = "charged"
  gen.add_evtgen_generator(finalstate=finalstate, path=path)

  # Define skim
  skim = feiHadronic(
      analysisGlobaltag=ma.getAnalysisGlobaltag(),
      OutputFileName="test_smartbkg_fei.udst.root"
  )

  # Add SmartBkg filtering by providing the skim code and final state
  gen.add_smartbkg_filtering(
      skim_code=skim.code,
      event_type=finalstate,
      path=path
  )

  # Add simulation and reconstruction
  sim.add_simulation(path)
  rec.add_reconstruction(path)

  # Apply the skim
  skim(path)

We currently provide a pre-trained model via the global tag ??? that is trained on 51 skims (for a full list see end of page). 

For studies you may want to disable filtering and look at the model output. This is possible by setting the ``debug_mode`` argument 
of :py:func:`generators.add_smartbkg_filtering` to ``True``. This will disable filtering and reweighting, and instead the 
model output will be saved to the event extra info as ``SmartBKG_Prediction``. An example of how to write out the model
predictions as well as the skim flags is provided under ``generators/examples/SmartBkg_NEW_DebugMode.py``. 

For greater customisability you may also use the :b2:mod:`SmartBackground` module directly. It has the same mandatory arguments 
as the convenience function, and can also be put into debug mode. It performs the reweighting, but no filtering on its own (instead 
it returns 1 as a return value if an event is sampled, otherwise 0). 

Legacy:

Using graph neural network with attention mechanism to predict whether a generated event will
pass the skim after detector simulation and reconstruction. Selection and weighting methods are
then invested to choose proper events according to their scores while trying to avoid bias.
The computational resource used for steps between generation and skim will be saved in this way.

For the inference part, a well trained neural network
(parameters saved in global tag ``SmartBKG_GATGAP`` with payload ``GATGAPgen.pth``)
to filter out events that can pass FEI hadronic B0 skim is available. The corresponding neural network
built with PyTorch is stored in ``generators/scripts/smartBKG/models/gatgap.py`` while
``generators/scripts/smartBKG/b2modules/NN_filter_module.py`` is a wrapper (``basf2.module``) suited in ``basf2``
framework. An example of how to generate skimmed events using SmartBKG can be found in 
``generators/examples/SmartBKGEvtGen.py``.

.. note:: Notice that each event generated through SmartBKG should be reweighted with the inversed neural network output.

To train the neural network for a different skim, you can use the provided example as a guide. 
First refer to ``generators/examples/SmartBKGSkimFlag.py`` and substitute the FEI hadronic B0 skim with 
your custom skimming process.

.. code-block:: python
    
    # Create the mDST output file before skimming
    mdst.add_mdst_output(
        path=main,
        filename=f'{out_dir}_submdst{job_id}.root',
    )
    # Arbitrary skimming process
    your_skimming(main)
    # Save the event number of each pass event as the flag for the training of NN
    main.add_module(SaveFlag(f'{out_dir}_flag{job_id}.parquet'))

Next, execute the script ``generators/examples/SmartBKGDataProduction.py`` to prepare training data. 
You can choose between fast mode and advanced mode by setting the ``save_vars`` argument to ``False`` or ``True``, 
respectively. In advanced mode, manual specifications are required in the scripts 
``generators/scripts/smartBKG/b2modules/NN_trainer_module/data_production.py`` and 
``generators/script/smartBKG/__init__.py`` for expected particle lists and variables. 
The provided example (``save_vars`` has to be given as ``True``, otherwise ``False`` by default) 
demonstrates the advanced mode, showcasing variables from ``Y(4S)`` and ``B`` lists. Intermediate files 
are automatically generated and removed during the advanced mode process, while in fast mode, 
each step is completed only in the cache, offering the advantage of less processing time and reduced disk load. 
The modules are designed to handle continuum datasets as well.

Both flag generation and preprocessing can be seamlessly executed on the same node of the batch system by 
specifying file names and/or job IDs.

.. note:: A substantial number of passing events (O(10^5)) is necessary for effective training of the neural network.

The neural network can be trained using ``generators/examples/SmartBKGTrain.py`` without the need for the ``basf2`` 
environment. This allows you to conduct training or fine-tuning locally on your GPU device, enhancing performance 
and speeding up the process.

To utilize your well-trained model locally, you can set the ``model_file`` parameter for the ``NNFilterModule``. 
Alternatively, you can update the globaltag 
(refer to :numref:`framework/doc/development/tuppr:Updating the main globaltag`).

Modules in this project:

.. autoclass:: smartBKG.b2modules.NN_trainer_module.SaveFlag
.. autoclass:: smartBKG.b2modules.NN_trainer_module.TrainDataSaver
.. autoclass:: smartBKG.b2modules.NN_trainer_module.data_production
