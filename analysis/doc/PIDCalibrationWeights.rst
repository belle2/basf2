.. _PIDCalibrationWeights:

PID Calibration Weights
=======================

The PID calibration weights were introduced in 
`BELLE2-NOTE-TE-2021-27 <https://docs.belle2.org/record/2721>`_ as a novel means
for improving PID performance without low-level variables.

The gist of the method is to perform a weighted sum of detector log-likelihoods
instead of a straight sum, with the weights to be trained using standard machine
learning techniques.

To facilitate this, we provide a script for training these weights as well as a
number of methods for preparing data samples for training and for analyzing the
resulting performance.


Preparing Data Samples for Training
-----------------------------------

In order to train weights on a data sample, we first need to prepare the data by
saving out only the information necessary for training. This is done with several
methods that are provided in the ``pidDataUtils`` module.

1. Read your data into a DataFrame. We provide a ``read_root()`` method to do
   this (it is just a wrapper around the uproot.concatenate method,
   which can read several files and automatically concatenate them all
   together.) You can use whatever method you like, though. All that is required
   is that your DataFrame **must** contain momentum ('p'), cosine-theta
   ('cosTheta'), phi ('phi'), and detector log-likelihood data for any particles
   of interest.
2. For each particle type of interest, identify the tag that serves as a prefix
   in the column names, and use the ``make_h5()`` method to make a *slim* H5
   file containing only the track and log-likelihood information for this
   particle type. For example, in a D* analysis, the tag for kaons is
   'DST_D0_K', and for pions the tags are 'DST_D0_pi' and 'DST_pi'.  There are
   two nuances here to beware of, however.

   1. If your DataFrame is from simulation and contains the 'mcPDG' column, you 
      can provide the argument ``pdg=None`` and the 'pdg' column in the HDF5
      file will be filled with the 'mcPDG' data. However, if you'd prefer to use
      the kinematic tags *or* you don't have 'mcPDG' data, provide the PDG code
      for the particle type you're extracting. (e.g. ``pdg=321`` for kaons,
      ``pdg=211`` for pions, etc.)
   2. This method assumes that the detector log-likelihood columns are formatted
      as f"pidLogLikelyhoodOf{pdg}From{detector}". If the log-likelihood columns
      in your dataset are named differently, please provide a function to the
      ``column`` argument to yield these column names. The function should take
      two arguments: the particle name ('e', 'mu', 'pi', 'K', 'p', 'd') and the
      detector name ('SVD', 'CDC', 'TOP', 'ARICH', 'ECL', 'KLM'). It should
      return the corresponding detector log-likelihood column name as a string.

3. Once slim H5 files are made for each particle type, merge them together using
   the ``merge_h5s()`` method. This method has a ``pdgs`` argument. If
   ``pdgs=None``, the method will simply use the values from the 'pdg' columns
   in the given H5 files. One can also give a list of PDG values, one per
   filename, to be used instead. (If you set the PDG values in the last step,
   this shouldn't be necessary. If you filled the columns with the mcPDG values,
   however, and at *this* stage would prefer to use kinematic tags, this is how
   you could achieve it.)
4. Use the ``split_h5()`` method to finalize preparation by splitting the slim
   H5 file into train, validation, and test sets. (Or only train and validation
   sets, if ``test_size=0``.) Note that the arguments ``train_size``,
   ``val_size``, and ``test_size`` need not sum to 1; the sizes will be
   renormalized if not.

Here is an example code snippet which we used to do this for a D* dataset.

.. code-block:: python

    import pidDataUtils as pdu

    df = pdu.read_root(['dstar_1.root', 'dstar_2.root'])
    pdu.make_h5(df, 'DST_D0_K', 'slim_dstar_K.h5', pdg=321)
    pdu.make_h5(df, ['DST_D0_pi', 'DST_pi'], 'slim_dstar_pi.h5', pdg=211)
    pdu.merge_h5s(['slim_dstar_K.h5', 'slim_dstar_pi.h5'], 'slim_dstar.h5')
    pdu.split_h5('slim_dstar.h5', 'slim_dstar/')


Training the Weights
--------------------

To train the weights, we provide a script: ``pidTrainWeights.py``. It can be run
using

.. code-block:: bash

    python3 pidTrainWeights.py data/ models/net.pt -n 100

In this snippet, we assume that ``split_h5()`` has been run and the output files
were written into the folder ``data/``. We then specify that we want our final
model to be saved to ``models/net.pt`` and that we want the model to be trained
for 100 epochs. Note that just the six-by-six array of weights will also be 
written to ``models/net_wgt.npy``.

.. note::
    The output filename needs to end in ``.pt`` or else the script will fail 
    when attempting to write the weights to a ``.npy`` file.

To train a model on the training dataset in ``data/`` but only over events
within a certain interval in momentum and theta, one can specify the limits at
the command-line. For example, to train only on events with momentum between 0.5
and 1.5 GeV and theta between 15 and 45 degrees, one could add the arguments
``--p_lims 0.5 1.5 --theta_lims 15 45``.

One can also start training from an existing checkpoint with this script by
using the ``--resume`` argument. There are three cases to consider with this
argument.

1. Omitting ``--resume`` means that a fresh set of weights will be trained
   and written to the output filepath, overwriting any existing model at that
   location.
2. Including only ``--resume`` (as a flag) means that the network at the 
   output filepath will be loaded, trained, and saved again, overwriting the 
   model at the output location.
3. Including ``--resume path/to/existing/model.pt`` allows one to load and train
   the network at the given filepath, but the final model is saved to the output
   location (so the existing model is not overwritten).

A fresh, new network is initialized with all weights equal to 1 by default. One 
can instead start the network with weights sampled from a normal distribution of
mean 1 and width 0.5 if desired by using the ``--random`` flag.

Lastly, one can limit the allowed particle types using the ``--only`` flag. For
example, if we are studying D* decays and only care about pions and kaons, and 
therefore want the non-{pi, K} particle types to have zero weight in the PID
computations, we can specify ``--only 211 321`` to zero and freeze the weights 
for the other hypotheses. Not specifying ``--only`` means that all particle
types will be used.

Below, you can find the full documentation for this script.

.. argparse::
    :filename: analysis/scripts/pidTrainWeights.py
    :func: get_parser
    :prog: pidTrainWeights.py
    :nodefault:



Applying Weights to Data for Performance Analysis
-------------------------------------------------

We also, through ``pidDataUtils`` provide methods for *applying* a set of
trained weights to a data sample for analysis. Here's what that workflow might
look like.

1. Read your data into a DataFrame. Much like when preparing data samples, you 
   can use our ``read_root()`` method. We also provide ``read_h5()`` and
   ``read_npz()``, which will read in the slim format H5 files or the train,
   validation, or test set ``npz`` files used for training.
2. Use ``prepare_df()`` to prepare the data. This method will apply weights to 
   the detector log-likelihoods and create a number of additional, useful
   columns in the DataFrame. These columns include likelihood ratios, binary
   likelihood ratios, single-detector and ablation PID, contribution metrics,
   and more. It does, however, have several arguments and features to be aware
   of.

   1. The weights are specified with the ``weights`` keyword argument. This 
      argument expects a six-by-six NumPy array (which could be obtained by
      using ``np.load()`` on the ``_wgt.npy`` file produced during training) or
      a dict, if there are individual weights for various momentum and theta 
      bins. The dict should have int-tuples as keys and six-by-six NumPy arrays 
      as values, where the int-tuple keys are ``(p_bin, theta_bin)``.
   2. Even if you are not using per-bin weights, you should still specify 
      momentum and theta bins for the analysis. These are done by giving 1D 
      NumPy arrays to the ``p_bins`` and ``theta_bins`` keyword arguments. By 
      default, the bins will be the standard Belle II systematics bins. Any
      events with momentum or theta outside of the bins will have their
      ``'p_bin'`` or ``'theta_bin'`` value set to -1. By default, these events
      are then cut from the DataFrame, but this can be disabled by setting 
      ``drop_outside_bins=False``.
   3. If there are certain particle types that you want to exclude entirely
      from PID computations, you can do so by setting a list of allowed particles 
      with the ``allowed_particles`` keyword argument. This expects a list of 
      particle *names* (not PDG codes).
   4. As with the ``make_h5()`` function, there is again a keyword argument
      ``column`` for the case where your DataFrame contains detector
      log-likelihoods that are named in a special way. For this function, it is
      assumed by default that the detector log-likelihoods are simply named 
      f"{detector}_{particle}", since that is the format used when we read in 
      the slim h5 or npz files. If you're using a ROOT file that was read with 
      ``read_root()`` and has columns in the format
      f"pidLogLikelyhoodOf{pdg}From{detector}", use ``column=root_column``.
      Otherwise, make your own function and give it here.

After using ``prepare_df()``, you should have a DataFrame with a number of
additional columns, including labels, likelihood ratios, binary likelihood
ratios, PID predictions, single-detector and ablation PID predictions, momentum
and theta bins, and contribution metrics. Feel free to analyze these however you 
like; however, there exists a Python package that expects DataFrames with these 
specific columns and produces a wide range of plots. It is ``pidplots``, which 
can be found `here <https://gitlab.desy.de/connor.hainje/pidanalysis>`_.


PID calibration weights on the basf2 path
-----------------------------------------

The PID calibration weights can be registered in the database to utilize them on
the basf2 path. The module :b2:mod:`PIDCalibrationWeightCreator` can produce the
dbobject PIDCalibrationWeight with a unique name of the weight matrix.
One can find an example of the usage of the module in
``analysis/examples/PIDCalibration/02_SamplePIDAnalysis.py``.

By loading the data object, the basf2 variables, such as
:b2:var:`weightedElectronID`, provide the weighted PID probability from the
original likelihood and the given data object. One can specify the name of the
weight matrix in the argument of the variables.

.. code-block:: python

   import basf2 as b2
   import modularAnalysis as ma

   # create path
   my_path = b2.create_path()

   # load the local dbobject
   localDB = 'localdb/database.txt'
   b2.conditions.append_testing_payloads(localDB)
   # or use the central global tag including the dbobject

   ma.fillParticleList('pi+:all', cut='', path=my_path)

   matrixName = "PIDCalibrationWeight_Example"
   ma.variablesToNtuple('pi+:all', ['pionID', 'weightedPionID('+matrixName+')'], path=my_path)





pidDataUtils Functions
----------------------

.. automodule:: pidDataUtils
    :members:
