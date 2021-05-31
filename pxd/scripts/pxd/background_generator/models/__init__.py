##
# @package pxd.background_generator.models
# This package encapsulates different generator models that can be
# selected to generate digits in the PXD background generator module.
"""
This package encapsulates different PXD background generator models.
"""

from importlib import import_module
from typing import Callable, Type


##
# Helper function that imports and returns the class `Model`
# from the Python module named as @p model and assumed to be
# located in the package `pxd.background_generator.models`.
#
# An error is raised if the Python module named as `model`:
#     * does not exist,
#     * does not contain the class `Model`.
#
# @param model: Model name and the name of the
#     corresponding Python module implementing the model
def _get_model_cls(model: str) -> Type:
    cls = None
    try:
        cls = import_module(f".{model}", __name__).Model
    except AttributeError as exc:
        exc.msg = f"please define a model class for {model!r}"
        raise
    except ModuleNotFoundError as exc:
        exc.msg = f"the selected model {model!r} is not implemented"
        raise
    return cls


##
# Helper function that imports and returns the function `generate`
# from the Python module named as @p model and assumed to be
# located in the package `pxd.background_generator.models`.
#
# An error is raised if the Python module named as `model`:
#     * does not exist,
#     * does not contain the function `generate`.
#
# @param model: Model name and the name of the
#     corresponding Python module implementing the model
def _get_generate_func(model: str) -> Callable:
    func = None
    try:
        func = import_module(f".{model}", __name__).generate
    except AttributeError as exc:
        exc.msg = f"please define the generation function for {model!r}"
        raise
    except ModuleNotFoundError as exc:
        exc.msg = f"the selected model {model!r} is not implemented"
        raise
    return func


##
# Container for names of generator models that are available for selection.
#
# Each model name is assumed to match a corresponding Python module that
# is located in the package `pxd.background_generator.models` and
# implements the specific model, i.e. defines a:
#     * class `Model`,
#     * function `generate`.
#
# The class `Model` must inherit from the base class `torch.nn.Module`.
#
# The generation function `generate` specific to a model is assumed
# to act on a model instance and produce an output of type `torch.Tensor`
# that is of shape `(40, 250, 768)` and contains values of type `torch.uint8`
# in the interval \f$ [0, 255] \f$. Each index in the first axis of the
# output tensor is assumed to correspond to the PXD sensor specified
# by the `VxdID` arguments in `pxd.background_generator.VXDID_ARGS`
# at the same index.
MODELS = (
    "convnet",
    "resnet",
)
