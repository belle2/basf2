.. _priorProbabilities:

Prior Probabilities
===================

This program uses the momentum and cos(theta) of particles to train a
machine learning model to calculate prior probabilities which are
particle identification probabilities before taking into account the
detector signals. Combining this with PID's from detector signals gives
posterior probabilities which can help improve particle identification.


Training
--------

Training requires as input an ntuple to train the model with, which must
contain data on particle momentum and cos(theta) as well as set of
particle PDG values which are to be considered for training. It also
requires path (and filename) to store model and scaling file (optional).
There are some options to modify other paramters for training as well.
The use flags for training are as follows: 



.. list-table:: **Required Flags**
   :widths: 1 1
   :header-rows: 1

   * - Flag
     - Usage
   * - -i, --input_path
     - Path to the ntuple for training

   * - -k, --key
     - Key of the tree to be used for training in the root file
   * - -o, --output_path
     - Output model file name (with path) end with ".pth"
   * - -p, --particle_list
     - List of particle mcPDG to be used (comma separated)

.. list-table:: **Optional Flags**
   :widths: 1 1 
   :header-rows: 1
   
   * - Flag
     - Usage
   * - -h, --help 
     - show help message and exit
   * - -r, --retrain_model
     - Path to the model for retraining
   * - -v, --variable_list
     - List of variable names in order cos(theta), momentum and pdg (default: cosTheta,p,mcPDG)
   * - -lr, --learning_rate
     - Learning rate for training (default = 1e-5)
   * - -e, --epochs
     - Number of epochs to be trained for (default = 64)
   * - -t, --taylor_terms
     - Number of terms of Taylor series of Cross Entropy Loss to be used during training. 
       Zero (0) denotes the use of direct log function which is the default
   * - -s, --scaling_file
     - Path to the root file to write data for scaling for calibration purpose
   
Usage
~~~~~

.. code-block ::

    TrainPriors.py -i ntuple.root -k data -o newmodel.pth -p 11,13,211,321 -r oldmodel.pth -v cosTheta,p,mcPDG -lr 1e-6 -e 100 -t 5 -s scale.root

.. note::
         - The output during the training process will show a 
           smaller value for validation loss but this is just because the loss on training
           set
           includes regularization.
         - In case you notice overfitting or want to stop the training, 
           Ctrl+C will stop the training and create the required output files so in
           case it takes a bit of time to exit out, it may be because it is writing
           those files. 

Evaluation
----------

For evaluation we have a class called Priors within EvalPriors. The user
needs to first initialize it using the trained model using an array of
particlelist, model and a scaling file if created. Then using relevant
input, we can get priors as well as posteriors.

Import
~~~~~~
.. code-block:: python

    from EvalPriors import Priors

Initialize
~~~~~~~~~~

.. code-block:: python

    prior = Priors(particle_list,model,scalefile)

Here, ``particle_list`` is an array of the form ``[11,13,211,321]`` and
``model`` and ``scalefile`` are paths to the model and scaling data file respectively which are of the form ``'/path/model_name.pth'`` and ``'/path/scalefile.root'`` respectively. However, the use of scaling file for calibration is optional.

.. note ::
          To use the scaling file for calibration, you are required to additionally install the netcal module using pip.

Prior Calculation
~~~~~~~~~~~~~~~~~

.. code-block:: python

    prior.calculatePriors(momentum,cosTheta)

Here, ``momentum`` and ``cosTheta`` are arrays.

Getting the priors
~~~~~~~~~~~~~~~~~~
.. code-block:: python

    prior.getPriors(pdg)

This returns a 1D array of calculated priors for the given PDG value.
However, this is an optional argument and specifying nothing returns a
2D array of priors arranged in ascending order of PDG values.

Getting posteriors
~~~~~~~~~~~~~~~~~~
.. code-block:: python

    prior.getPosterior(pid,pdg)

Again PDG is an optional argument but PID likelihoods must be provided
as 2D array containing likelihoods for the particles in
``particle_list`` with PDG values taken in **ascending order**.
	