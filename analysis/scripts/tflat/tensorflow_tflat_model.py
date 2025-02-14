#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import keras


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
    """
    num_tracks = parameters.get("num_tracks")
    num_features = parameters.get("num_features")
    num_transformer_blocks = parameters.get("num_transformer_blocks")
    num_heads = parameters.get("num_heads")
    embedding_dims = parameters.get("embedding_dims")
    mlp_hidden_units_factors = parameters.get("mlp_hidden_units_factors")
    dropout_rate = parameters.get("dropout_rate")

    # Create model inputs
    inputs = keras.layers.Input((number_of_features,))

    # Replace NaN's by a special number
    raw_features = keras.ops.nan_to_num(inputs, nan=9999)

    # 3D tensor with axes for samples, tracks and features
    reshaped_features = keras.layers.Reshape((num_tracks, num_features))(raw_features)

    # Create a keras mask for padded tracks
    masked_features = keras.layers.Masking(mask_value=9999)(reshaped_features)

    # Normalize the input features
    normed_features = keras.layers.BatchNormalization()(masked_features)

    # Embed features in latent space with embedding_dims
    encoded_features = keras.layers.Dense(
        units=embedding_dims,
        activation=keras.activations.selu,
        name="Embedding_dense_1")(normed_features)
    encoded_features = keras.layers.Dropout(dropout_rate, name="Embedding_dropout_1")(encoded_features)
    encoded_features = keras.layers.BatchNormalization(name="Embedding_batchnorm")(encoded_features)
    encoded_features = keras.layers.Dense(
        units=embedding_dims,
        activation=keras.activations.selu,
        name="Embedding_dense_2")(encoded_features)
    encoded_features = keras.layers.Dropout(dropout_rate, name="Embedding_dropout_2")(encoded_features)

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
        feedforward_output = keras.layers.Dense(units=3*embedding_dims, activation='relu',
                                                name=f"feedforward_{block_idx}_dense_1")(x)
        feedforward_output = keras.layers.Dense(units=embedding_dims, name=f"feedforward_{block_idx}_dense_2")(feedforward_output)
        feedforward_output = keras.layers.Dropout(dropout_rate, name=f"feedforward_{block_idx}_dropout")(feedforward_output)
        # Skip connection 2.
        x = keras.layers.Add(name=f"skip_connection2_{block_idx}")([feedforward_output, x])
        # Layer normalization 2.
        encoded_features = keras.layers.LayerNormalization(
            name=f"layer_norm2_{block_idx}", epsilon=1e-6
        )(x)

    # Pool the "contextualized" embeddings of the features.
    features = keras.layers.GlobalAveragePooling1D()(encoded_features)

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
        name="ClassifierMLP",
    )(features)

    # Add a sigmoid as a binary classifer.
    outputs = keras.layers.Dense(units=1, activation="sigmoid", name="sigmoid")(features)
    model = keras.Model(inputs=inputs, outputs=outputs)
    return model
