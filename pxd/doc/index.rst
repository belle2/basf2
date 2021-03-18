========================
PXD Background Generator
========================

Generate PXD background samples for background overlay on the fly.

The package provides a PXD background generator :py:class:`basf2.Module` module
that can be added to the module execution path to generate PXD digits on-the-fly
and store them into a background digit collection. The module is designed to
replace the collection stored by the :any:`!BGOverlayInput` module.

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


Usage
=====

Invoking the PXD background generator module
requires the PyTorch Python package. PyTorch
version 1.4.0 is included in the basf2 externals
v01-09-01, thus, no installation is necessary.

The PXD background generator module can be used
in simulation by first creating a :py:class:`.Specs` container object
with the generator specifications. This object is then passed
as an argument to the function :py:func:`simulation.add_simulation`
which will automatically add the PXD background generator module
to the execution path:

.. code-block:: python

    from background import get_background_files
    from pxd.background_generator import Specs
    from simulation import add_simulation

    files = get_background_files()
    specs = Specs(model='resnet')

    add_simulation(path, bkgfiles=files, pxd_background_generator=specs)

An example steering file for generating, simulating,
and reconstructing :math:`B\bar{B}` events
using the PXD background generator module is
provided in ``pxd/examples/background_generator.py``.


Reference
=========

.. automodule:: pxd.background_generator
    :members:
    :show-inheritance:
