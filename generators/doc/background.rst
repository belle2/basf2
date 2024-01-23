SmartBKG: Selective background simulation using graph neural networks
---------------------------------------------------------------------

Using graph neural network with attention mechanism to predict whether a generated event will
pass the skim after detector simulation and reconstruction. Selection and weighting methods are
then invested to choose proper events according to their scores while trying to avoid bias.
The computational resource used for steps between generation and skim will be saved in this way.

For the inference part, a well trained neural network
(parameters saved in global tag ``SmartBKG_GATGAP`` with payload ``GATGAPgen.pth``)
to filter out events that can pass FEI hadronic B0 skim is available. The corresponding neural network
built with PyTorch is stored in ``generators/scripts/smartBKG/models/gatgap.py`` while
``generators/scripts/smartBKG/b2modules/NN_filter_module.py`` is a wrapper (``basf2.module``) suited in basf2
framework. An example of how to generate skimmed events using SmartBKG can be found in 
``generators/examples/SmartBKGEvtGen.py``.

.. note:: Notice that each event generated through SmartBKG should be reweighted with the inversed neural network output.

To train the neural network for a different skim, you can use the provided example as a guide. 
Refer to ``generators/examples/SmartBKGSkimTracking.py`` and substitute the FEI hadronic B0 skim with 
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

Execute ``generators/examples/SmartBKGDataProduction.py`` following the completion of ``SmartBKGSkimTracking.py`` 
to prepare the training data. These two steps can be seamlessly executed on the same node of the batch system.

.. note:: A substantial number of passing events (O(10^5)) is necessary for effective training of the neural network.

The neural network can be trained using ``generators/examples/SmartBKGTrain.py`` without the need for the basf2 
environment. This allows you to conduct training or fine-tuning locally on your GPU device, enhancing performance 
and speeding up the process.

To utilize your well-trained model locally, you can set the ``model_file`` parameter for the ``NNFilterModule``. 
Alternatively, you can update the globaltag 
(refer to :numref:`framework/doc/development/tuppr:Updating the main globaltag`).

If you wish to apply the project to particle lists other than Y(4S) and B, manual specification is required 
in the script ``generators/scripts/smartBKG/b2modules/NN_trainer_module/data_production.py`` and 
``generators/script/smartBKG/__init__.py``
The modules are designed to handle continuum datasets as well.


Modules in this project:

.. autoclass:: smartBKG.b2modules.NN_trainer_module.SaveFlag
.. autoclass:: smartBKG.b2modules.NN_trainer_module.TrainDataSaver
.. autoclass:: smartBKG.b2modules.NN_trainer_module.data_production
.. autoclass:: smartBKG.b2modules.NN_filter_module.NNFilterModule  