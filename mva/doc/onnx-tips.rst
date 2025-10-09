ONNX tips and tricks
====================

This page provides some additional information on converting models to ONNX and working with ONNX in basf2.

Create an MVA payload with an ONNX model inside
-----------------------------------------------

If you created a new ONNX model (as opposed to converting an existing model) you can create an MVA weightfile using the following helper function:

.. autofunction:: basf2_mva_util.create_onnx_mva_weightfile

Converting an existing model
----------------------------

To convert a model in an existing MVA weightfile, the steps are

- download or open the original weightfile from the database
- extract the contained model
- (optional depending on model) modify the model to include pre- or postprocessing
- convert it to onnx
- (optional depending on model) modify the onnx model to include pre- or postprocessing
- create a new MVA weightfile with the ONNX model in it
- test if the outputs are consistent

The first two steps can be avoided if the original model and weights that were put into the MVA weightfile are still available.

Download and extract existing weight- and steering files
--------------------------------------------------------

To download/open and existing payload either use the command line tools for the :ref:`mva_condition_database` or the via the python API:

.. code:: python

    from basf2_mva import Weightfile

    weightfile = Weightfile().load(identifier)

where ``identifier`` can be a filename or a database identifier. For weightfiles using the Python MVA method we need to extract ``Python_Weightfile``, which is typically a pickled state object:

.. code:: python

    import pickle

    weightfile.getFile("Python_Weightfile", "Python_Weightfile.pkl")
    with open("Python_Weightfile.pkl", "rb") as f:
        obj = pickle.load(f)

What to do with this ``obj`` now depends on how the specific MVA Python ``framework`` is implemented. Have a look at the corresponding module in ``mva/scripts/basf2_mva_python_interface``.

If we convert a weightfile that uses the MVA Python method, in addition to the weights, we also should have a look at the steering file since it may have custom pre- or postprocessing defined:

.. code:: python

    weightfile.getFile("Python_Steeringfile", "Python_Steeringfile.pkl")
    with open("Python_Steeringfile.pkl", "rb") as f:
        steering = pickle.load(f)
    print(steering)

Most of the time custom code we need to consider is found in the ``apply`` function.

For ``torch`` it's most delicate since the whole model state, potentially including custom class definitions is pickled. Here we will have to ``exec`` the steering file in the ``torch`` MVA module namespace, e.g. to be able to unpickle models:

.. code:: python

    import basf2_mva_python_interface

    with open("Python_Steeringfile.py") as f:
        exec(f.read(), basf2_mva_python_interface.torch.__dict__)


For torch we can then load the model like this:

.. code:: python

    state = basf2_mva_python_interface.torch.load(obj)
    model = state.model

For other frameworks one can usually save model weights directly from ``obj`` (often contains lists of filenames and list of file data) and then use the corresponding library (e.g. tensorflow, keras, xgboost, ...) to load the weights into a model.


Custom pre- and postprocessing
------------------------------

ONNX provides a large number of `operators <https://onnx.ai/onnx/operators/>`__ - which are typically enough to represent any transformation steps like standardization, splitting or concatenating tensors and reshaping operations.

The easiest approach in NN libraries is usually to add these operations before converting to ONNX since the libraries, e.g. torch or tensorflow, themselves provide enough means to add these steps.

If this is not possible, e.g. because the transformation was done in a different library another approach is to convert the transformation and the model separately to ONNX models and merge them.

Finally one can manually craft and modify ONNX models, e.g. using `onnx.helper <https://onnx.ai/onnx/api/helper.html>`__. For adding larger sets of custom operations, `onnxscript <https://github.com/microsoft/onnxscript>`__ offers an alternative with less boilerplate code.

Torch example
^^^^^^^^^^^^^

In torch one fast way is to wrap the existing model into the forward pass of a new model, for example applying a scaling:

.. code:: python

    import torch
    from torch import nn

    torch_model = ... # code loading the original model
    torch_model.eval()

    offset = ...
    scale = ...

    class ONNXModel(nn.Module):
        def forward(self, x):
            x = x - offset
            x = x / scale
            return torch_model(x)

    inp = torch.rand(1, 16) # assume 16 variables as input

    with torch.no_grad():
        torch.onnx.export(
            ONNXModel(),
            inp,
            "model.onnx",
            input_names=["input"],
            output_names=["output"],
            dynamo=True,
            external_data=False,
        )


Keras example
^^^^^^^^^^^^^

In keras we can use the functional API (in ``keras>3`` use `keras.ops <https://keras.io/api/ops/>`__ for non-trivial operations) to add additional steps before or after the model:

.. code:: python

    import keras
    import tensorflow as tf

    model = ... # code loading the original model

    offset = ...
    scale = ...

    def get_model_with_transformation(model):
        inp = keras.layers.Input((16,)) # assume 16 variables as input
        x = inp
        x = x - offset
        x = x / scale
        x = model(x)
        return keras.models.Model(inputs=[inp], outputs=[x])

    new_model = get_model_with_transformation(model)

    inp = tf.random.normal((1, 16))
    new_model(inp) # call once to build model

    new_model.export("model.onnx", format="onnx")


Merging models
^^^^^^^^^^^^^^

Also mention onnxscript here

Graph networks
--------------

Using ONNX standalone
---------------------

Prototyping in python
^^^^^^^^^^^^^^^^^^^^^

Write a basf2 c++ module
^^^^^^^^^^^^^^^^^^^^^^^^

General notes
-------------

- Use the latest versions of converters. While ONNX operations follow a clear scheme and are backwards compatible, the ecosystem around converters is evolving fast and steadily. Use venvs outside basf2 or b2venv for running model conversions.
- Test your resulting ONNX models. Feeding in random input values and comparing the results against the original model is often a good first check.
- chatGPT/LLMs together with the docs of libraries help to navigate the space of possibilities for different conversion tools and model surgery.
