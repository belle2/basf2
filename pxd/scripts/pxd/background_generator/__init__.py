##
# @package pxd.background_generator
# Generate PXD background samples for background overlay on the fly.
"""
Generate PXD background samples for background overlay on the fly.
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
# Sequence of forty arguments represented as
# tuples with elements `(layer, ladder, sensor)` used to
# instantiate `VxdID` objects that collectively specify all distinct PXD sensors.
VXDID_ARGS = tuple(
    tuple(product([1], [ladder + 1 for ladder in range(8)], [1, 2]))
    + tuple(product([2], [ladder + 1 for ladder in range(12)], [1, 2]))
)


##
# Class for container objects with specifications for
# invoking the PXD background generator module.
class Specs:
    """Create a container object with specifications for
    invoking the PXD background generator module.

    :param model: Name of the generator model - either "convnet" or "resnet",
        defaults to "convnet"
    :type model: str, optional

    :param checkpoint: Path to the checkpoint file containing the model weights,
        defaults to None - download the checkpoint from the conditions database
    :type checkpoint: str, optional

    :param seed: Integer number in the range of :math:`[-2^{63}, 2^{63} - 1]`
        used as the initial seed for the generator,
        defaults to None - derive deterministically from the
        value returned by :py:func:`basf2.get_random_seed`
    :type seed: int, optional

    :param nintra: Number of intra-op threads to be utilized for the generation,
        defaults to 1
    :type nintra: int, optional

    :param ninter: Number of inter-op threads to be utilized for the generation,
        defaults to 1
    :type ninter: int, optional

    :param globaltag: Global tag in the conditions database
        providing the payloads with checkpoint files,
        defaults to "PXDBackgroundGenerator"
    :type globaltag: str, optional
    """

    ##
    # Static method used to verify that `model`:
    #     * is a string,
    #     * is a valid name for a model that is available for selection.
    #
    # The value of `model` is returned if the conditions are met.
    # An exception is raised otherwise.
    @staticmethod
    def _validate_model(model: str) -> str:
        if not isinstance(model, str):
            raise TypeError("expecting type str `model`")
        elif model not in MODELS:
            options = ", ".join(map(repr, MODELS))
            raise ValueError(f"invalid `model`: {model!r} (options: {options}")
        return model

    ##
    # Static method used to verify that `checkpoint`:
    #     * is either None, a string, or a `pathlib.Path` object,
    #     * is a valid path to an existing file - if not None.
    #
    # The value of `checkpoint` is returned if the conditions are met.
    # An exception is raised otherwise.
    @staticmethod
    def _validate_checkpoint(checkpoint: Union[None, str, pathlib.Path]) -> str:
        if not isinstance(checkpoint, (type(None), str, pathlib.Path)):
            raise TypeError("expecting None or type str `checkpoint`")
        if checkpoint is None:
            return checkpoint
        checkpoint = os.path.expandvars(str(checkpoint))
        if not (os.path.exists(checkpoint) and os.path.isfile(checkpoint)):
            raise ValueError(f"invalid `checkpoint`: {checkpoint!r}")
        return checkpoint

    ##
    # Static method used to verify that `seed`:
    #     * is either None or an integer,
    #     * is in the interval \f$ [-2^{63}, 2^{63} - 1] \f$ - if not None.
    #
    # The value of `seed` is returned if the conditions are met.
    # An exception is raised otherwise.
    @staticmethod
    def _validate_seed(seed: Union[None, int]) -> Union[None, int]:
        if not isinstance(seed, (type(None), int)):
            raise TypeError("expecting None or type int `seed`")
        if seed is None:
            return seed
        if not -(2 ** 63) <= seed < 2 ** 63:
            raise ValueError(f"expecting -2^63 <= `seed` < 2^63 (got: {seed})")
        return seed

    ##
    # Static method used to verify that `nintra`:
    #     * is an integer,
    #     * is larger than zero.
    #
    # The value of `nintra` is returned if the conditions are met.
    # An exception is raised otherwise.
    @staticmethod
    def _validate_nintra(nintra: int) -> int:
        if not isinstance(nintra, int):
            raise TypeError("expecting type int `nintra`")
        elif not nintra > 0:
            raise ValueError(f"expecting `nintra` > 0 (got: {nintra}")
        return nintra

    ##
    # Static method used to verify that `ninter`:
    #     * is an integer,
    #     * is larger than zero.
    #
    # The value of `ninter` is returned if the conditions are met.
    # An exception is raised otherwise.
    @staticmethod
    def _validate_ninter(ninter: int) -> int:
        if not isinstance(ninter, int):
            raise TypeError("expecting type int `ninter`")
        elif not ninter > 0:
            raise ValueError(f"expecting `ninter` > 0 (got: {ninter}")
        return ninter

    ##
    # Static method used to verify that `globaltag`:
    #     * is a string.
    #
    # The value of `globaltag` is returned if the condition is met.
    # An exception is raised otherwise.
    @staticmethod
    def _validate_globaltag(globaltag: str) -> str:
        if not isinstance(globaltag, str):
            raise TypeError("expecting type str `globaltag`")
        return globaltag

    ##
    # Create a container objects with specifications for
    # invoking the PXD background generator module.
    #
    # @param model: Name of the generator model - either "convnet" or "resnet",
    #     defaults to "convnet"
    #
    # @param checkpoint: Path to the checkpoint file containing the model weights,
    #     defaults to None - download the checkpoint from the conditions database
    #
    # @param seed: Integer number in the range of \f$ [-2^{63}, 2^{63} - 1] \f$
    #     used as the initial seed for the generator,
    #     defaults to None - derive deterministically from the
    #     value returned by `basf2.get_random_seed()`
    #
    # @param nintra: Number of intra-op threads to be utilized for the generation,
    #     defaults to 1
    #
    # @param ninter: Number of inter-op threads to be utilized for the generation,
    #     defaults to 1
    #
    # @param globaltag: Global tag in the conditions database
    #     providing the payloads with checkpoint files,
    #     defaults to "PXDBackgroundGenerator"
    def __init__(
        self,
        model: str = "convnet",
        checkpoint: Union[None, str, pathlib.Path] = None,
        seed: Union[None, int] = None,
        nintra: int = 1,
        ninter: int = 1,
        globaltag: str = "PXDBackgroundGenerator",
    ):
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

    ##
    # Method that returns the initialized generator model instance
    # and the corresponding generation function.
    def _instantiate(self):
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
    # Path to the checkpoint file containing the model weights

    ##
    # @var seed
    # Integer number used as the initial seed for the generator

    ##
    # @var nintra
    # Number of intra-op threads to be utilized for the generation

    ##
    # @var ninter
    # Number of inter-op threads to be utilized for the generation

    ##
    # @var globaltag
    # Global tag in the conditions database
    # providing the payloads with checkpoint files


##
# Class for the PXD background generator module.
class PXDBackgroundGenerator(basf2.Module):
    """Generates PXD background samples on-the-fly.

    .. warning::
        PXD digits added to the background collection by a
        preceding module in the execution path are deleted.

    :param specs: Container object with the specifications
    :type specs: :py:class:`Specs`
    """

    ##
    # Constructor for the PXD background generator module.
    #
    # @param specs: Container object of type `Specs` with the specifications
    def __init__(self, specs: Specs):
        """"""
        super().__init__()
        if not isinstance(specs, Specs):
            raise TypeError(f"expecting type {Specs.__module__}.Specs `specs`")
        self.specs = specs

    ##
    # Method called before event processing.
    #
    # This method infers the name extension of the background digit collection,
    # instantiates the VxdID identifier objects required to create digits,
    # and instantiates and initializes the selected generator model.
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

    ##
    # Method called when processing an event.
    #
    # This method first clears the background digit collection
    # then proceeds to generate a batch of forty background images,
    # locates pixel hits in images, transcodes the pixel hits into digits,
    # and stores the digits into the background digit collection.
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

    ##
    # Method called after all events are processed.
    #
    # This method deletes the attribute reference to the generator instance.
    def terminate(self):
        """"""
        # delete the reference to the generator instance
        del self.generator

    ##
    # @var specs
    # Container object with the specifications

    ##
    # @var storearr
    # PXD background digit collection

    ##
    # @var vxdids
    # Sequence of `VxdID` objects that
    # collectively specify all distinct PXD sensors

    ##
    # @var generator
    # Generator model instance

    ##
    # @var generate_func
    # Generation function applied on the model instance to
    # return an output that is transcoded into digits


##
# Helper function that instantiates the PXD background generator module
# and adds the module instance to the execution path.
#
# @param path: Execution path to add the module instance to
# @param specs: Specifications for invoking the module
def add_pxd_background_generator(path: basf2.Path, specs: Specs = None):
    """Instantiate the PXD background generator module
    and add the module instance to the execution path.

    .. note::
        This function is intended to be invoked inside the
        function :py:func:`.add_simulation`.

    :param path: Execution path
    :type path: :py:class:`basf2.Path`

    :param specs: Specifications for invoking the module, defaults to None
    :type specs: :py:class:`Specs`, optional

    This function does not do anything if the ``specs`` argument is None.
    """
    if specs is not None:
        # instantiate the module and add it to the execution path
        basf2.B2INFO("Adding the PXD background generator module to path")
        path.add_module(PXDBackgroundGenerator(specs))

        # enable the global tag that stores the model checkpoints
        basf2.conditions.append_globaltag(specs.globaltag)
