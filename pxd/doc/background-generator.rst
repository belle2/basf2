Background Generator
--------------------

The package :py:mod:`!pxd.background_generator` implements a proof-of-concept generator of PXD data for background overlay on the fly.
The generator is a stand-alone :py:class:`.PXDBackgroundGenerator` module integrated into the current background overlay setup.
Each instance of background overlay in detector simulation requires input data to combine with simulated signal data.
Background overlay input data --- a background sample of detector outputs --- is high in volume.
Computing sites must obtain background overlay input samples before data production.
Assuming the correlation between the output of the PXD and other detectors in background samples is negligible, the volume of data distributed is reduced by using generated PXD output.

The module is designed to be incorporated into the detector simulation and used to provide partial background overlay input data in conjunction with data from background overlay files.


Usage
+++++

.. warning::

    The module is a proof of concept not intended for production.

Background overlay is enabled by providing one or more files with input samples.
In the current setup, background overlay uses exclusively data from the background overlay files:

.. code-block:: python

    import basf2
    from background import get_background_files
    from simulation import add_simulation
    
    path = basf2.create_path()
    path.add_module("EventInfoSetter", evtNumList=[128])
    path.add_module("EvtGenInput")
    add_simulation(path, bkgfiles=get_background_files())
    basf2.process(path)

The module is integrated to discard the PXD output from the sampled data and provide background overlay with new input generated on the fly.
To use the module, create a new module instance.
Note that a Python environment with `PyTorch`_ is required --- it is recommended to use the environment included with the externals.
Use the drop-in replacement function incorporating the module instance instead of the standard function to initialize the simulation:

.. code-block:: python
    :emphasize-lines: 3,5,6

    import basf2
    from background import get_background_files
    from pxd.background_generator import PXDBackgroundGenerator, inject_simulation
    
    module = PXDBackgroundGenerator(model="resnet")
    add_simulation = inject_simulation(module)
    
    path = basf2.create_path()
    path.add_module("EventInfoSetter", evtNumList=[128])
    path.add_module("EvtGenInput")
    add_simulation(path, bkgfiles=get_background_files())
    basf2.process(path)

.. seealso::

    Example steering file provided in ``$BELLE2_RELEASE_DIR/pxd/examples/background_generator.py``.

The module implements different generator models.
A module instance uses one of the implemented generator models.

.. _PyTorch: https://www.pytorch.org


Models
++++++

The generator is based on generative machine learning models pre-trained on samples of PXD output from background overlay files.
The implemented generator models ``convnet`` and ``resnet`` are neural networks of different architectures inspired by `WGAN-GP`_.
To initialize a generator model, a checkpoint file with pre-trained model parameters must be provided.
If no checkpoint is specified, the module transfers and uses the default checkpoint from the conditions database.

The PXD consists of forty modules, referred to as sensors.
Each sensor embeds a :math:`250 \times 768` pixel matrix.
Sensors are approximated to have pixels with matching dimensions and equally spaced.
This approximation is necessary to allow the same representation of all sensor readout by grayscale images --- heat maps.
Accordingly, a PXD readout and output corresponds to forty grayscale images.
Training instances of the default checkpoints for both models are based on the same training dataset of :math:`3.6 \times 10^6` images.
The images were extracted from nine background overlay files with release version ``01-00-00`` with simulated background samples.
In the future, it is hoped that more checkpoints with model parameters trained on different background datasets will be provided to generate run-dependent samples.

Generally, a PXD output is produced from forty images specific for each sensor.
The generator models currently implemented generate one image at a time and do not allow specifying the sensor for which the image is to be generated.
For this reason, the module is limited to produce PXD output from forty arbitrary images, some belonging to the same sensor.
The limitation is expected to be resolved with the implementation of new class-conditional models that are currently being developed.

.. _WGAN-GP: https://arxiv.org/abs/1704.00028


Development
+++++++++++

.. rubric:: Model Implementation

To implement a new generator model, choose a unique model name and add the name into the collection :py:const:`!pxd.background_generator.models.MODELS`.
Create a Python module with the same name in the package :py:mod:`!pxd.background_generator.models`.
The module must contain the definition of a model class and a generation function.

The model class defines the neural network model structure --- it must be named ``Model`` and inherit from the :py:class:`!torch.nn.Module` base class.
The generation function defines how to use a model instance to generate a PXD output sample --- it must be named ``generate``, act on a model instance, and return a :py:class:`!torch.Tensor` with shape ``(40, 250, 768)`` and :py:class:`!torch.uint8` values.


Reference
+++++++++

.. autoclass:: pxd.background_generator.PXDBackgroundGenerator

.. autofunction:: pxd.background_generator.inject_simulation
