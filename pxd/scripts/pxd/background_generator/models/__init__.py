"""
This package encapsulates different PXD background generator models.

To implement your own generator model, first create a new Python
module named after your model. Define a class named `Model`
inheriting from the `torch.nn.Module` base class and
place it your Python module. Next, define a function
named `generate` which acts on an instance of your model and
produces a `torch.Tensor` output of type `torch.uint8`,
shape `(40, 250, 768)`, and containing values in :math:`[0, 255]`.
The output of this function is transcoded into pixel hits and digits.
Finally, add the name of your Python module
to the tuple :py:const:`MODELS`.

"""

from importlib import import_module
from typing import Callable, Type


def _get_model_cls(model: str) -> Type:
    """"""
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


def _get_generate_func(model: str) -> Callable:
    """"""
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
# container for model names (should be the same as module names)
MODELS = (
    "convnet",
    "resnet",
)
