"""
========================
PXD Background Generator
========================

Generate PXD background data on-the-fly using a pre-trained generator model.

The package provides a PXD background generator :py:class:`basf2.Module` module
that can be added to the module execution path to generate PXD digits on-the-fly
and store them into a background digit collection. The module is designed to
replace the collection stored by the :any:`!BGOverlayInput` module.

Two proof-of-concept generator models are currently implemented and
can be used to generate PXD background data. They are based on the
machine learning framework Generative Adversarial Network (GAN),
specifically, the Wasserstein GAN framework which uses the
gradient penalty. The models have been trained on a dataset
consisting of :math:`3.6 \\times 10^6` grayscale images of
the same height and width :math:`250 \\times 768`. Each image
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
in simulation by first creating a :py:class:`Specs` container object
with the generator specifications. This object is then passed
as an argument to the function :py:func:`simulation.add_simulation`
which will automatically add the PXD background generator module
to the execution path:

    >>> from pxd.background_generator import Specs
    >>> from simulation import add_simulation
    >>> specs = Specs(model='resnet', seed=0)
    >>> simulation.add_simulation(path, pxd_background_generator=specs)

An example steering file for generating, simulating,
and reconstructing :math:`B\\bar{B}` events
using the PXD background generator module is
provided in ``pxd/examples/background_generator.py``.

"""
import hashlib
import os.path
import pathlib
from itertools import product
from secrets import randbelow
from typing import Union

import basf2
from ROOT.Belle2 import DataStore, PyStoreArray, PyStoreObj
from ROOT.Belle2 import DBAccessorBase, DBStoreEntry
from ROOT.Belle2 import VxdID, PXDDigit

from .models import MODELS
from .models import _get_model_cls, _get_generate_func

##
# VxdID arguments describing all distinct PXD sensors; format: (layer, ladder, sensor)
VXDID_ARGS = tuple(
    tuple(product([1], [ladder + 1 for ladder in range(8)], [1, 2]))
    + tuple(product([2], [ladder + 1 for ladder in range(12)], [1, 2]))
)


class Specs:
    """Create a container for the generator specifications
    that is to be passed to the PXD background generator module.

    :param model: Name of the generator model - either 'convnet' or 'resnet',
        defaults to 'convnet'
    :type model: str, optional
    :param checkpoint: Path to the file with the pre-trained model weights,
        defaults to None - download the checkpoint automatically
    :type checkpoint: str, optional
    :param seed: Integer number :math:`[-2^{63}, 2^{63} - 1]`
        used as the initial seed for the internal pseudo-random number generator,
        defaults to None - derive a deterministic seed from the basf2 seed
    :type seed: int, optional
    :param nintra: Number of intra-op threads to be utilized for the generation,
        defaults to 1
    :type nintra: int, optional
    :param ninter: Number of inter-op threads to be utilized for the generation,
        defaults to 1
    :type ninter: int, optional
    :param globaltag: Global tag in the conditions database
        which is used to download the model checkpoints automatically,
        defaults to 'PXDBackgroundGenerator'
    :type globaltag: str, optional
    """

    @staticmethod
    def _validate_model(model: str) -> str:
        """"""
        if not isinstance(model, str):
            raise TypeError("expecting type str `model`")
        elif model not in MODELS:
            options = ", ".join(map(repr, MODELS))
            raise ValueError(f"invalid `model`: {model!r} (options: {options}")
        return model

    @staticmethod
    def _validate_checkpoint(checkpoint: Union[None, str, pathlib.Path]) -> str:
        """"""
        if not isinstance(checkpoint, (type(None), str, pathlib.Path)):
            raise TypeError("expecting None or type str `checkpoint`")
        if checkpoint is None:
            return checkpoint
        checkpoint = os.path.expandvars(str(checkpoint))
        if not (os.path.exists(checkpoint) and os.path.isfile(checkpoint)):
            raise ValueError(f"invalid `checkpoint`: {checkpoint!r}")
        return checkpoint

    @staticmethod
    def _validate_seed(seed: Union[None, int]) -> Union[None, int]:
        """"""
        if not isinstance(seed, (type(None), int)):
            raise TypeError("expecting None or type int `seed`")
        if seed is None:
            return seed
        if not -(2 ** 63) <= seed < 2 ** 63:
            raise ValueError(f"expecting -2^63 <= `seed` < 2^63 (got: {seed})")
        return seed

    @staticmethod
    def _validate_nintra(nintra: int) -> int:
        """"""
        if not isinstance(nintra, int):
            raise TypeError("expecting type int `nintra`")
        elif not nintra > 0:
            raise ValueError(f"expecting `nintra` > 0 (got: {nintra}")
        return nintra

    @staticmethod
    def _validate_ninter(ninter: int) -> int:
        """"""
        if not isinstance(ninter, int):
            raise TypeError("expecting type int `ninter`")
        elif not ninter > 0:
            raise ValueError(f"expecting `ninter` > 0 (got: {ninter}")
        return ninter

    @staticmethod
    def _validate_globaltag(globaltag: str) -> str:
        """"""
        if not isinstance(globaltag, str):
            raise TypeError("expecting type str `globaltag`")
        return globaltag

    def __init__(
        self,
        model: str = "convnet",
        checkpoint: Union[None, str, pathlib.Path] = None,
        seed: Union[None, int] = None,
        nintra: int = 1,
        ninter: int = 1,
        globaltag: str = "PXDBackgroundGenerator",
    ):
        """"""
        # process `model`
        self.model = type(self)._validate_model(model)

        # process `checkpoint`
        self.checkpoint = type(self)._validate_checkpoint(checkpoint)

        # process `seed`
        self.seed = type(self)._validate_seed(seed)

        # process `nintra`
        self.nintra = type(self)._validate_nintra(nintra)

        # process `ninter`
        self.ninter = type(self)._validate_ninter(ninter)

        # process `globaltag`
        self.globaltag = type(self)._validate_globaltag(globaltag)

    def _instantiate(self):
        """"""
        try:
            import torch
        except ImportError as exc:
            exc.msg = "please install PyTorch: `pip3 install torch==1.4.0`"
            raise

        # derive a deterministic seed from the basf2 seed - if none given
        if self.seed is None:
            obj = basf2.get_random_seed()
            func = hashlib.sha512()
            func.update(bytes(str(obj), "utf-8"))
            digest = func.digest()[:8]
            self.seed = int.from_bytes(digest, "big", signed=True)

        # load the checkpoint from the conditions database - if none given
        if self.checkpoint is None:
            payload_name = f"PXDBackgroundGenerator_{self.model}"
            accessor = DBAccessorBase(DBStoreEntry.c_RawFile, payload_name, True)
            self.checkpoint = accessor.getFilename()

        # set the number of inter-op CPU threads
        torch.set_num_interop_threads(self.ninter)

        # set the number of intra-op CPU threads
        torch.set_num_threads(self.nintra)

        # import the generation function
        generate_func = _get_generate_func(self.model)

        # import the model class and instantiate the model
        generator = _get_model_cls(self.model)()

        # restore the model weights from the checkpoint
        generator.load_state_dict(torch.load(self.checkpoint, map_location="cpu"))

        # set the operational mode to inference
        generator.eval()

        # disable the computation of gradients
        for param in generator.parameters():
            param.requires_grad = False

        # initialize the seed of the internal pseudo-random number generator
        basf2.B2INFO(f"PXD background generator seed initialized to {self.seed}")
        torch.manual_seed(self.seed)

        # return the instantiated model and the generation function as a tuple
        return generator, generate_func

    ##
    # @var model
    # Name of the generator model

    ##
    # @var checkpoint
    # Path to the file with the pre-trained model weights

    ##
    # @var seed
    # Integer number used as the initial seed for the
    # internal pseudo-random number generator

    ##
    # @var nintra
    # Number of intra-op threads to be utilized for the generation

    ##
    # @var ninter
    # Number of inter-op threads to be utilized for the generation

    ##
    # @var globaltag
    # Global tag in the conditions database which is used to
    # download the model checkpoints automatically


class PXDBackgroundGenerator(basf2.Module):
    """Generates PXD background data on-the-fly.

    .. warning::
        PXD digits added to the background collection in the DataStore
        prior to the execution of this module are deleted.

    :param specs: Generator specifications container
    :type specs: :py:class:`Specs`
    """

    def __init__(self, specs: Specs):
        """"""
        super().__init__()
        if not isinstance(specs, Specs):
            raise TypeError(f"expecting type {Specs.__module__}.Specs `specs`")
        self.specs = specs

    def initialize(self):
        """"""
        # infer the name extension of the background digit colection
        bkginfo = PyStoreObj("BackgroundInfo", DataStore.c_Persistent)
        if not bkginfo.isValid():
            # no background digit collection found
            basf2.B2ERROR("path must contain the BGOverlayInput module")
        extension = bkginfo.getExtensionName()

        # register the background digit collection
        self.storearr = PyStoreArray("PXDDigits", DataStore.c_DontWriteOut)
        self.storearr.registerInDataStore(f"PXDDigits{extension}")

        # instantiate all distinct sensor identifier objects
        self.vxdids = tuple(VxdID(arg) for arg in VXDID_ARGS)

        # initialize the generator and the generation function per the specifications
        self.generator, self.generate_func = self.specs._instantiate()

    def event(self):
        """"""
        # cache variable references
        vxdids = self.vxdids
        storearr = self.storearr
        storearr_ptr = self.storearr.getPtr()

        # clear any existing digits in the collections
        storearr_ptr.Clear()

        # generate one background images for each sensor
        x = self.generate_func(self.generator)

        # find nonzero pixel indices that correspond to pixel hits
        nonzero = x.nonzero(as_tuple=True)
        args = [indices.tolist() for indices in nonzero]
        vals = x[nonzero].tolist()

        # transcode into digits
        for n, (idx, ucell, vcell, charge) in enumerate(zip(*args, vals)):
            # pointer to a digit with the default constructor arguments
            storearr.appendNew()
            # pointer to a new digit with custom arguments
            storearr_ptr[n] = PXDDigit(vxdids[idx], ucell, vcell, charge)

        # delete variable references
        del x, nonzero, args, vals

    def terminate(self):
        """"""
        # delete the reference to the generator instance
        del self.generator

    ##
    # @var specs
    # Generator specifications container

    ##
    # @var storearr
    # Digit collection

    ##
    # @var vxdids
    # VxdID objects describing all distinct PXD sensors

    ##
    # @var generator
    # Generator model instance

    ##
    # @var generate_func
    # Generation function


def add_pxd_background_generator(path: basf2.Path, specs: Specs = None):
    """Add the PXD background generator module to the execution path.

    :param path: Module execution path
    :type path: :py:class:`basf2.Path`
    :param specs: Generator specifications, defaults to None
    :type specs: :py:class:`Specs`, optional
    """
    if specs is not None:
        # add the module to the path
        basf2.B2INFO("Adding the PXD background generator module to path")
        path.add_module(PXDBackgroundGenerator(specs))

        # enable the global tag that stores the model checkpoints
        basf2.conditions.append_globaltag(specs.globaltag)
