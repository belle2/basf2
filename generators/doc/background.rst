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

The neural network can be trained using ``generators/examples/SmartBKGTrain.py`` without the need for the basf2 
environment. This allows you to conduct training or fine-tuning locally on your GPU device, enhancing performance 
and speeding up the process.

To utilize your well-trained model locally, you can set the ``model_file`` parameter for the ``NNFilterModule``. 
Alternatively, you can update the globaltag 
(refer to :numref:`framework/doc/development/tuppr:Updating the main globaltag`).

Modules in this project:

.. autoclass:: smartBKG.b2modules.NN_trainer_module.SaveFlag
.. autoclass:: smartBKG.b2modules.NN_trainer_module.TrainDataSaver
.. autoclass:: smartBKG.b2modules.NN_trainer_module.data_production
