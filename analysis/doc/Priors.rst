.. _PidPriorProbabilities:

PID Prior Probabilities
=======================

This tool uses the momentum and cos(:math:`\theta`) of particles to train a
machine learning model to calculate prior probabilities which are
particle identification probabilities before taking into account the
detector signals. Combining this with PID's from detector signals gives
posterior probabilities which can help improve particle identification.


Training
--------

.. argparse::
    :filename: analysis/tools/analysis-train-priors
    :func: get_parser
    :prog: analysis-train-priors
    :nodefault:

.. note::
         - The output during the training process will show a 
           smaller value for validation loss but this is just because the loss on training
           set includes regularization.
         - In case you notice overfitting or want to stop the training, 
           Ctrl+C will stop the training and create the required output files so in
           case it takes some time to exit out, it may be because it is writing
           those files.

.. warning::
         - The tool does not consider the particles with PDG values other than the ones given 
           in ``particle_list`` (even as background) during training so it is advisable to include 
           particles with sufficiently large proportions in the ``particle_list`` even if their 
           priors are not required.
         - In case your ``particle_list`` contains only two particles, kindly avoid creation
           of scaling file because ``TemperatureScaling`` which is used for calibration
           of priors will give out only the output for the higher PDG value and so further 
           functions like posterior calculation (as well as getting priors for a specific PDG 
           value) will not work.

Evaluation
----------

For evaluation we have a class called Priors within EvalPriors. The user
needs to first initialize it using the trained model, the particlelist for 
which the model was trained and a scaling file (if created). Then using relevant
input, we can get priors as well as posteriors.

Import
~~~~~~
.. code-block:: python

    from evalPriors import Priors

Initialize
~~~~~~~~~~

.. code-block:: python

    prior = Priors(particle_list,model,scalefile)

Here, ``particle_list`` is a list of the form ``[11,13,211,321]`` and
``model`` and ``scalefile`` are paths to the model and scaling data file 
respectively which are of the form ``'/path/model_name.pth'`` and 
``'/path/scalefile.root'`` respectively. However, the use of scaling file 
for calibration is optional.

.. note::
          To use the scaling file for calibration, you are required to 
          additionally install the netcal module using pip.

Prior Calculation
~~~~~~~~~~~~~~~~~

.. code-block:: python

    prior.calculate_priors(momentum,cosTheta)

Here, ``momentum`` and ``cosTheta`` are numpy arrays.

Getting the Priors
~~~~~~~~~~~~~~~~~~

.. code-block:: python

    prior.get_priors(pdg)

This returns a 1D array of calculated priors for the given ``pdg`` value.
However, this is an optional argument and specifying nothing returns a
2D array of priors arranged in ascending order of PDG values.

Getting the Posteriors
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

    prior.get_posterior(pid,pdg)

Again ``pdg`` is an optional argument but ``pid`` likelihoods must be provided
as 2D array containing likelihoods for the particles in
``particle_list`` with PDG values taken in **ascending order**.
	
