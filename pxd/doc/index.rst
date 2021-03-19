========================
PXD Background Generator
========================

The package :py:mod:`!pxd.background_generator` provides a generator of PXD background samples for background overlay on the fly.
Background generation on the fly is motivated to reduce the data volume in background overlay files currently distributed.

The proof-of-concept generator is implemented as a stand-alone :py:class:`.PXDBackgroundGenerator` module designed to build on the existing background overlay setup.
The module uses one of the generator models available for selection to generate PXD digits on the fly and store the generated digits into the background collection.
The generated digits replace the existing digits in the background digit collection previously samlped from the background overlay files.


Usage
=====

The module is integrated into the detector simulation with background overlay.
By default, background overlay uses data sampled from the background overlay files for all detectors.


.. note::

    Invoking the module requires a Python environment with PyTorch.


To enable the module and use the generated PXD digits instead,
create a container object with the specifications for invoking the module and
pass the object to the :py:func:`simulation.add_simulation` function as the keyword argument ``pxd_background_generator``:


.. code-block:: python

    from background import get_background_files
    from pxd.background_generator import Specs
    from simulation import add_simulation

    files = get_background_files()
    specs = Specs(model='resnet')

    add_simulation(path, bkgfiles=files, pxd_background_generator=specs)


The module can use different generator models.
The model can be selected with the keyword argument ``model`` in the :py:class:`.Specs` instance used to invoke the module.

.. seealso::
    An example steering file is available at ``pxd/examples/background_generator.py``.


Models
======

Two proof-of-concept generator models are currently implemented and
can be used to generate PXD background data. They are based on the
machine learning framework Generative Adversarial Network (GAN),
specifically, the Wasserstein GAN framework which uses the
gradient penalty. The models have been trained on a dataset
consisting of :math:`3.6 \times 10^6` grayscale images of
the same height and width :math:`250 \times 768`. Each image
represents a readout of one PXD sensor (a "PXD module") in
an event extracted from a subset of background overlay files of
the release version 01-00-00. The models are designed to generate
one image at a time which can be transcoded into pixel hits
and digits describing one PXD sensor.

Generating a PXD background event requires the generator to
produce forty images, each specific to one distinct PXD sensor.
In this case, the batch of generated images yields pixel hits
which collectively describe one full PXD readout.
In the initial implementation of this module,
it is approximated that a PXD background event is produced
from a generated batch of non-specific sensor images,
in other words, it is assumed that all PXD sensors
are the same. This approximation will not be made in
in future models, specifically, models which will
allow class-conditional image generation.

.. rubric:: Implementing a new model

Choose a unique model name to add into the :py:const:`!pxd.background_generator.models.MODELS` collection,
create a Python module with the same name in the :py:mod:`!pxd.background_generator.models` package.

The module shall include a model class and a generation function named ``Model`` and ``generate``, respectively.
The model class must inherit from the ``torch.nn.Module`` base class.
The generation function must act on a model instance and return a ``torch.Tensor`` with shape ``(40, 250, 768)`` and values of type ``torch.uint8`` in the range :math:`[0, 255]`. 


Reference
=========

.. autoclass:: pxd.background_generator.PXDBackgroundGenerator

.. autoclass:: pxd.background_generator.Specs
