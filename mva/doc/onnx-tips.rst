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
- convert it to ONNX
- (optional depending on model) modify the ONNX model to include pre- or postprocessing
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

For ``torch`` it's most delicate since the whole model state, potentially including custom class definitions, is pickled. Here we will have to ``exec`` the steering file in the ``torch`` MVA module namespace, e.g. to be able to unpickle models:

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


Merging models (onnx/onnxscript example)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Suppose you already have a trained model in ONNX format. We can implement the same prescaling as above by directly constructing an ONNX ModelProto 
and merging with the provided one:

.. code:: python

    import onnx
    from onnxscript import script, FLOAT
    from onnxscript import opset22

    offset = ...
    scale = ...

    # Load the existing model
    model = onnx.load("path/to/model.onnx")

    # Construct scaling transformation using the script decorator from onnxscript,
    # we assume here the loaded model has one input of size 10, N is the batch dimension
    @script(default_opset=opset22)
    def prescale(X: FLOAT["N", 10]) -> FLOAT["N", 10]:
        return (X - offset) / scale
    
    prescale_model = prescale.to_model_proto()

    # Merge prescale and actual model by providing mapping between prescaling output 
    # (default name 'return_val') and model input (here assumed to be called 'input')
    combined_model = onnx.compose.merge_models(
        prescale_model,
        model,
        io_map=[("return_val", "input")],
        name="scaled_model"
    )

    onnx.save(combined_model, "path/to/output.onnx")


ONNX in a basf2 C++ module
--------------------------

In case you want/need to run inference of an ONNX model in a C++ basf2 module, the MVA package provides a simple interface which should be 
sufficient for most use-cases. To run your inference, first you need to fetch your ONNX model from the conditions database. We recommend 
saving the model as a raw file into the database, such that it can simply be accessed like this:

.. code:: cpp

    auto accessor = DBAccessorBase(DBStoreEntry::c_RawFile, payloadName, true);
    const std::string filename = accessor.getFilename();

where ``payloadName`` is the name of the payload storing the raw ONNX file. Now, you can initialize an ONNX inference session:

.. code:: cpp

    std::unique_ptr<MVA::ONNX::Session> m_session;
    m_session = std::make_unique<MVA::ONNX::Session>(filename.c_str());

where we recommend keeping the session pointer as a member variable of your module if you intend to run the inference multiple times.

Now you need to fill all required input tensors for your model. You can create input tensors in different ways, for example by providing 
a flat vector that contains all values, and a vector describing the shape of the tensor:

.. code:: cpp

    // Create a 2 by 2 tensor of int32_t values from a flat vector of length 4
    std::vector<int32_t> inputs = {1, 2, 3, 4};
    auto inputTensor = MVA::ONNX::Tensor<int32_t>::make_shared(inputs, {2, 2});

Alternatively, you can also create an empty tensor and fill it element by element:

.. code:: cpp

    // Create a 2 by 2 tensor of int32_t values by manually setting the elements
    auto inputTensor = MVA::ONNX::Tensor<int32_t>::make_shared({2, 2});
    for (int i=0; i<4; i++) {
      inputTensor->at(i) = i;
    }

The ``at`` method works both with flat indices (when given a single value) or with tensor indices (when given a vector of values).
You also need an output tensor to capture the inference results:

.. code:: cpp

    // Create an output tensor for an ONNX model with 10 float outputs
    auto outputTensor = MVA::ONNX::Tensor<float>::make_shared({10});

Finally, you can run inference simply by calling the ``run`` method providing a map of input/output names to the appropriate tensors:

.. code:: cpp

    // Run inference on a model with two inputs 'input1' and 'input2' and one output 'output' by providing the corresponding tensors
    m_session->run({{"input1", inputTensor1}, {"input2", inputTensor2}}, {{"output", outputTensor}});


General notes
-------------

- Use the latest versions of converters. While ONNX operations follow a clear scheme and are backwards compatible, the ecosystem around converters is evolving fast and steadily. Use venvs outside basf2 or b2venv for running model conversions.
- Test your resulting ONNX models. Feeding in random input values and comparing the results against the original model is often a good first check.
- chatGPT/LLMs together with the docs of libraries help to navigate the space of possibilities for different conversion tools and model surgery.
