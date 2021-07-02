##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
##
# @package pxd.background_generator
# Generate PXD background samples for background overlay on the fly.
"""
Generate PXD background samples for background overlay on the fly.
"""
import functools
import hashlib
import os.path
import pathlib
from itertools import product
from secrets import randbelow
from typing import Callable, Union

import basf2
from ROOT.Belle2 import DataStore, PyStoreArray, PyStoreObj
from ROOT.Belle2 import DBAccessorBase, DBStoreEntry
from ROOT.Belle2 import VxdID, PXDDigit

from .models import MODELS
from .models import _get_model_cls, _get_generate_func

##
# Sequence of forty tuples `(layer, ladder, sensor)` used to
# instantiate `VxdID` specifier objects for distinct PXD modules.
#
# It is assumed that the indices of tuples in the sequence match
# indices along the first axis of tensors with shape `(40, 250, 768)`
# that are produced by the generator.
VXDID_ARGS = tuple(
    tuple(product([1], [ladder + 1 for ladder in range(8)], [1, 2]))
    + tuple(product([2], [ladder + 1 for ladder in range(12)], [1, 2]))
)


##
# Function to verify that `model`:
#     * is a string,
#     * is a valid name for a model that is available for selection.
#
# The value of `model` is returned if the conditions are met.
# An exception is raised otherwise.
def _verify_model(model: str) -> str:
    if not isinstance(model, str):
        raise TypeError("expecting type str `model`")
    elif model not in MODELS:
        options = ", ".join(map(repr, MODELS))
        raise ValueError(f"invalid `model`: {model!r} (options: {options}")
    return model


##
# Function to verify that `checkpoint`:
#     * is either None, a string, or a `pathlib.Path` object,
#     * is a valid path to an existing file - if not None.
#
# The value of `checkpoint` is returned if the conditions are met.
# An exception is raised otherwise.
def _verify_checkpoint(checkpoint: Union[None, str, pathlib.Path]) -> str:
    if not isinstance(checkpoint, (type(None), str, pathlib.Path)):
        raise TypeError("expecting None or type str `checkpoint`")
    if checkpoint is None:
        return checkpoint
    checkpoint = os.path.expandvars(str(checkpoint))
    if not (os.path.exists(checkpoint) and os.path.isfile(checkpoint)):
        raise ValueError(f"invalid `checkpoint`: {checkpoint!r}")
    return checkpoint


##
# Function to verify that `seed`:
#     * is either None or an integer,
#     * is in the interval \f$ [-2^{63}, 2^{63} - 1] \f$ - if not None.
#
# The value of `seed` is returned if the conditions are met.
# An exception is raised otherwise.
def _verify_seed(seed: Union[None, int]) -> Union[None, int]:
    if not isinstance(seed, (type(None), int)):
        raise TypeError("expecting None or type int `seed`")
    if seed is None:
        return seed
    if not -(2 ** 63) <= seed < 2 ** 63:
        raise ValueError(f"expecting -2^63 <= `seed` < 2^63 (got: {seed})")
    return seed


##
# Function to verify that `nintra`:
#     * is an integer,
#     * is larger than zero.
#
# The value of `nintra` is returned if the conditions are met.
# An exception is raised otherwise.
def _verify_nintra(nintra: int) -> int:
    if not isinstance(nintra, int):
        raise TypeError("expecting type int `nintra`")
    elif not nintra > 0:
        raise ValueError(f"expecting `nintra` > 0 (got: {nintra}")
    return nintra


##
# Function to verify that `ninter`:
#     * is an integer,
#     * is larger than zero.
#
# The value of `ninter` is returned if the conditions are met.
# An exception is raised otherwise.
def _verify_ninter(ninter: int) -> int:
    if not isinstance(ninter, int):
        raise TypeError("expecting type int `ninter`")
    elif not ninter > 0:
        raise ValueError(f"expecting `ninter` > 0 (got: {ninter}")
    return ninter


##
# Function to verify that `globaltag`:
#     * is a string.
#
# The value of `globaltag` is returned if the condition is met.
# An exception is raised otherwise.
def _verify_globaltag(globaltag: str) -> str:
    if not isinstance(globaltag, str):
        raise TypeError("expecting type str `globaltag`")
    return globaltag


##
# Class for the PXD background generator module.
class PXDBackgroundGenerator(basf2.Module):
    """Generates PXD background samples for background overlay on the fly.

    :param model: Name of the generator model to use - either "convnet" or "resnet",
        defaults to "convnet"
    :type model: str, optional

    :param checkpoint: Path to the checkpoint file with weights for the selected model,
        defaults to None - use the default checkpoint from the conditions database
    :type checkpoint: str, optional

    :param seed: Integer number in the interval :math:`[-2^{63}, 2^{63} - 1]`
        used internally as the initial seed,
        defaults to None - derive a deterministic seed from the
        value returned by :py:func:`basf2.get_random_seed`
    :type seed: int, optional

    :param nintra: Number of intra-op threads to be utilized for the generation,
        defaults to 1
    :type nintra: int, optional

    :param ninter: Number of inter-op threads to be utilized for the generation,
        defaults to 1
    :type ninter: int, optional

    :param globaltag: Global tag of the conditions database
        providing the default checkpoints stored as payloads,
        defaults to "PXDBackgroundGenerator"
    :type globaltag: str, optional
    """

    ##
    # Constructor for the PXD background generator module.
    #
    # @param model: Name of the generator model to use - either "convnet" or "resnet",
    #     defaults to "convnet"
    #
    # @param checkpoint: Path to the checkpoint file with weights for the selected model,
    #     defaults to None - use the default checkpoint from the conditions database
    #
    # @param seed: Integer number in the interval \f$ [-2^{63}, 2^{63} - 1] \f$
    #     used internally as the initial seed,
    #     defaults to None - derive a deterministic seed from the
    #     value returned by `basf2.get_random_seed()`
    #
    # @param nintra: Number of intra-op threads to be utilized for the generation,
    #     defaults to 1
    #
    # @param ninter: Number of inter-op threads to be utilized for the generation,
    #     defaults to 1
    #
    # @param globaltag: Global tag of the conditions database
    #     providing the default checkpoints stored as payloads,
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
        super().__init__()
        # process `model`
        self.model = _verify_model(model)

        # process `checkpoint`
        self.checkpoint = _verify_checkpoint(checkpoint)

        # process `seed`
        self.seed = _verify_seed(seed)

        # process `nintra`
        self.nintra = _verify_nintra(nintra)

        # process `ninter`
        self.ninter = _verify_ninter(ninter)

        # process `globaltag`
        self.globaltag = _verify_globaltag(globaltag)

        # enable the specified global tag
        basf2.conditions.append_globaltag(self.globaltag)

    ##
    # Method called before event processing to initialize the module.
    def initialize(self):
        try:
            import torch
        except ImportError as exc:
            exc.msg = "please install PyTorch: `pip3 install torch==1.4.0`"
            raise

        # set the number of inter-op CPU threads
        torch.set_num_interop_threads(self.ninter)

        # set the number of intra-op CPU threads
        torch.set_num_threads(self.nintra)

        # get the generation function for the selected model
        self._generate_func = _get_generate_func(self.model)

        # instantiate the generator model
        self._generator = _get_model_cls(self.model)()

        # initialize the model weights
        checkpoint = self.checkpoint
        if self.checkpoint is None:
            # use the default checkpoint from the conditions database
            payload = f"PXDBackgroundGenerator_{self.model}"
            accessor = DBAccessorBase(DBStoreEntry.c_RawFile, payload, True)
            checkpoint = accessor.getFilename()
        self._generator.load_state_dict(torch.load(checkpoint, map_location="cpu"))

        # set mode of operation to inference
        self._generator.eval()

        # disable the computation of gradients
        for param in self._generator.parameters():
            param.requires_grad = False

        # initialize the seed
        seed = self.seed
        if seed is None:
            # derive from the basf2 seed value
            obj = basf2.get_random_seed()
            func = hashlib.sha512()
            func.update(bytes(str(obj), "utf-8"))
            digest = func.digest()[:8]
            seed = int.from_bytes(digest, "big", signed=True)
        basf2.B2INFO(f"PXD background generator seed initialized to {seed}")
        torch.manual_seed(seed)

        # instantiate objects for specifying distinct PXD modules
        self._vxdids = tuple(VxdID(arg) for arg in VXDID_ARGS)

        # get the name of the extension used by BGOverlayInput for background collections
        bkginfo = PyStoreObj("BackgroundInfo", DataStore.c_Persistent)
        if not bkginfo.isValid():
            # no information about background overlay input available
            basf2.B2ERROR("path must contain the BGOverlayInput module")
        extension = bkginfo.getExtensionName()

        # register the PXD background digit collection - array - in the data store
        self._pystorearray = PyStoreArray("PXDDigits", DataStore.c_DontWriteOut)
        self._pystorearray.registerInDataStore(f"PXDDigits{extension}")

    ##
    # Method called each time an event is processed.
    def event(self):
        # get the low-level array accessor
        digit_array = self._pystorearray.getPtr()

        # clear digits stored by BGOverlayInput
        digit_array.Clear()

        # generate a batch of images - one image for each PXD module
        x = self._generate_func(self._generator)

        # locate indices of pixels with non-zero values - pixel hits
        nonzero = x.nonzero(as_tuple=True)
        args = [indices.tolist() for indices in nonzero]
        vals = x[nonzero].tolist()

        # store indices and pixel values into the data store as background digits
        for n, (idx, ucell, vcell, charge) in enumerate(zip(*args, vals)):
            # append a new default digit to expand the array
            self._pystorearray.appendNew()
            # modify the array to point to the correct digit
            digit_array[n] = PXDDigit(self._vxdids[idx], ucell, vcell, charge)

        # delete references to release memory
        del x, nonzero, args, vals

    ##
    # @var model
    # Name of the generator model

    ##
    # @var checkpoint
    # Path to the checkpoint file with the pre-trained model weights

    ##
    # @var seed
    # Integer number in the interval \f$ [-2^{63}, 2^{63} - 1] \f$
    # set as the initial seed

    ##
    # @var nintra
    # Number of intra-op threads utilized

    ##
    # @var ninter
    # Number of inter-op threads utilized

    ##
    # @var globaltag
    # Global tag of the conditions database
    # providing the default checkpoints stored as payloads

    ##
    # @var _generator
    # Generator model instance

    ##
    # @var _generate_func
    # Generation function applied on the model instance to
    # return an output that is transcoded into digits

    ##
    # @var _pystorearray
    # Accessor for PXD background digits in the data store

    ##
    # @var _vxdids
    # Sequence of identifier objects for each PXD module


##
# Helper function to incorporate a module instance
# into `add_simulation` after `BGOverlayInput`.
#
# @param module: Module instance to be incorporated,
#     defaults to None - return unmodified function
#
# @return Drop-in replacement function for `add_simulation`
def inject_simulation(module: Union[None, basf2.Module] = None) -> Callable:
    """Incorporate a module instance
    into :py:func:`.add_simulation` after `!BGOverlayInput`.

    :param module: Module instance to be incorporated,
        defaults to None - return unmodified function
    :type module: :py:class:`basf2.Module`, optional

    :returns: Drop-in replacement function for :py:func:`.add_simulation`
    """
    from simulation import add_simulation

    if module is None:
        # no modifications necessary
        return add_simulation
    elif not isinstance(module, basf2.Module):
        raise TypeError("expecting None or type basf2.Module `module`")

    @functools.wraps(add_simulation)
    def injected_simulation(path, *args, **kwargs):
        # create a separate path with simulation modules
        simulation_path = basf2.Path()
        add_simulation(simulation_path, *args, **kwargs)

        # manually add the simulation modules to the given path
        for simulation_module in simulation_path.modules():
            # append the next module from the simulation path
            path.add_module(simulation_module)

            if simulation_module.name() == "BGOverlayInput":
                # incorporate the given module
                path.add_module(module)

    return injected_simulation
