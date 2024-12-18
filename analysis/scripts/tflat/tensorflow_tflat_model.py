#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from keras.layers import Input, Discretization
import keras
import numpy as np

from functools import partial

num_bins = 64


class Slicing(keras.layers.Layer):
    def __init__(self, column, **kwargs):
        super().__init__(**kwargs)
        self.column = column

    def call(self, inputs):
        return inputs[:, self.column:self.column+1]

    def get_config(self):
        config = super().get_config()
        config.update({"column": self.column})
        return config


class NanToNum(keras.layers.Layer):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    def call(self, inputs):
        return keras.ops.nan_to_num(inputs)

    def get_config(self):
        config = super().get_config()
        return config


def get_preprocessor(X):
    """
    Configure and adapt preprocessor on data X
    """
    number_of_features = X.shape[1]
    inputs = Input(shape=(number_of_features,))

    outputs = {}
    for col in range(number_of_features):
        Xcol = X[:, col]
        disc_layer = Discretization(num_bins=num_bins, epsilon=0.0001,)
        disc_layer.adapt(Xcol[~np.isnan(Xcol)])

        # Note: After calling adapt(), both `num_bins` and `bin_boundaries`
        # attributes are not None. This causes a ValueError when deserializing
        # the Discretization layer later. So we manually force `num_bins` to None.
        disc_layer.num_bins = None

        net = Slicing(col)(inputs)
        net = disc_layer(net)
        outputs["{}".format(col)] = net

    return keras.models.Model(inputs=inputs, outputs=outputs)


def create_model_inputs(number_of_features):
    inputs = {}
    for col in range(number_of_features):
        feature_name = "{}".format(col)
        inputs[feature_name] = keras.layers.Input(name=feature_name, shape=(1,), dtype="int64")

    return inputs


def encode_inputs(inputs, embedding_dims):
    encoded_feature_list = []

    for feature_name in inputs:
        # Create an embedding layer with the specified dimensions.
        embedding = keras.layers.Embedding(
            input_dim=num_bins, output_dim=embedding_dims
        )
        # Convert the index values to embedding representations.
        encoded_feature = embedding(inputs[feature_name])
        encoded_feature_list.append(encoded_feature)

    return encoded_feature_list


def create_mlp(hidden_units, dropout_rate, activation, normalization_layer, name=None):
    mlp_layers = []
    for units in hidden_units:
        mlp_layers.append(normalization_layer())
        mlp_layers.append(keras.layers.Dense(units, activation=activation))
        mlp_layers.append(keras.layers.Dropout(dropout_rate))

    return keras.Sequential(mlp_layers, name=name)


def get_tflat_model(parameters, number_of_features):
    """
    Configure tflat model from parameters

    The model is based on the paper
    "TabTransformer: Tabular Data Modeling Using Contextual Embeddings"
    https://arxiv.org/abs/2012.06678

    The implementation of the model is inspired from
    https://keras.io/examples/structured_data/tabtransformer/

    """
    num_transformer_blocks = 3
    num_heads = 4
    embedding_dims = 16
    mlp_hidden_units_factors = [2, 1,]
    dropout_rate = 0.2
    use_column_embedding = False

    # Create model inputs
    inputs = create_model_inputs(number_of_features)

    # Encode features
    encoded_feature_list = encode_inputs(inputs, embedding_dims)

    # Stack feature embeddings for the Tansformer.
    encoded_features = keras.ops.stack(encoded_feature_list, axis=1)

    # Add column embedding to categorical feature embeddings.
    if use_column_embedding:
        num_columns = encoded_features.shape[1]
        column_embedding = keras.layers.Embedding(
            input_dim=num_columns, output_dim=embedding_dims
        )
        column_indices = keras.ops.arange(start=0, stop=num_columns, step=1)
        encoded_features = encoded_features + column_embedding(
            column_indices
        )

    # Create multiple layers of the Transformer block.
    for block_idx in range(num_transformer_blocks):
        # Create a multi-head attention layer.
        attention_output = keras.layers.MultiHeadAttention(
            num_heads=num_heads,
            key_dim=embedding_dims,
            dropout=dropout_rate,
            name=f"multihead_attention_{block_idx}",
        )(encoded_features, encoded_features)
        # Skip connection 1.
        x = keras.layers.Add(name=f"skip_connection1_{block_idx}")(
            [attention_output, encoded_features]
        )
        # Layer normalization 1.
        x = keras.layers.LayerNormalization(name=f"layer_norm1_{block_idx}", epsilon=1e-6)(x)
        # Feedforward.
        feedforward_output = create_mlp(
            hidden_units=[embedding_dims],
            dropout_rate=dropout_rate,
            activation=keras.activations.gelu,
            normalization_layer=partial(
                keras.layers.LayerNormalization, epsilon=1e-6
            ),  # using partial to provide keyword arguments before initialization
            name=f"feedforward_{block_idx}",
        )(x)
        # Skip connection 2.
        x = keras.layers.Add(name=f"skip_connection2_{block_idx}")([feedforward_output, x])
        # Layer normalization 2.
        encoded_features = keras.layers.LayerNormalization(
            name=f"layer_norm2_{block_idx}", epsilon=1e-6
        )(x)

    # Flatten the "contextualized" embeddings of the features.
    features = keras.layers.Flatten()(encoded_features)

    # Compute MLP hidden_units.
    mlp_hidden_units = [
        factor * features.shape[-1] for factor in mlp_hidden_units_factors
    ]
    # Create final MLP.
    features = create_mlp(
        hidden_units=mlp_hidden_units,
        dropout_rate=dropout_rate,
        activation=keras.activations.selu,
        normalization_layer=keras.layers.BatchNormalization,
        name="MLP",
    )(features)

    # Add a sigmoid as a binary classifer.
    outputs = keras.layers.Dense(units=1, activation="sigmoid", name="sigmoid")(features)
    model = keras.Model(inputs=inputs, outputs=outputs)
    return model


def get_merged_model(preprocessor, tflat_model, number_of_features):
    """
    Create a keras model that includes the preprocessing step
    """
    inputs = Input(shape=(number_of_features,))
    outputs = preprocessor(inputs)
    outputs = tflat_model(outputs)
    return keras.models.Model(inputs=inputs, outputs=outputs)
