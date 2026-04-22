#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import keras


@keras.saving.register_keras_serializable(package="MyLayers")
class MyConcatenate(keras.layers.Layer):
    """Concatenate the 3D input tensors and their 2D masks along the axis=1 dimension."""

    def call(self, inputs):
        '''
        Expect the input to be list of 3D tensors
        '''
        return keras.ops.concatenate(inputs, axis=1)

    def compute_mask(self, inputs, mask=None):
        '''
        Expect the mask to be list of 2D mask tensors
        '''
        if mask is None:
            return None
        return keras.ops.concatenate(mask, axis=1)

    def get_config(self):
        '''
        Keras needs this
        '''
        return super().get_config()


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
    clip_value = parameters.get("clip_value")
    mask_value = parameters.get("mask_value")
    num_trk = parameters.get("num_trk")
    num_trk_features = parameters.get("num_trk_features")
    num_ecl = parameters.get("num_ecl")
    num_ecl_features = parameters.get("num_ecl_features")
    num_roe = parameters.get("num_roe")
    num_roe_features = parameters.get("num_roe_features")
    num_transformer_blocks = parameters.get("num_transformer_blocks")
    num_heads = parameters.get("num_heads")
    embedding_dims = parameters.get("embedding_dims")
    mlp_hidden_units_factors = parameters.get("mlp_hidden_units_factors")
    dropout_rate = parameters.get("dropout_rate")

    # Compute start columns for tracks, ecl clusters and rest of event
    trk_start = 0
    ecl_start = num_trk*num_trk_features
    roe_start = ecl_start + num_ecl*num_ecl_features

    # Create model inputs
    inputs = keras.layers.Input((number_of_features,))

    # Replace NaN's by a special number
    raw_features = keras.ops.nan_to_num(inputs, nan=mask_value)

    # Clip features to mitigate outliers
    raw_features = keras.ops.clip(raw_features, x_min=-clip_value, x_max=clip_value)

    # Preprocess the track features

    # Get the track features
    raw_trk_features = raw_features[:, trk_start:trk_start+num_trk*num_trk_features]

    # 3D tensor with axes for samples, tracks and features
    reshaped_trk_features = keras.layers.Reshape((num_trk, num_trk_features))(raw_trk_features)

    # Create a keras mask for padded tracks
    masked_trk_features = keras.layers.Masking(mask_value=mask_value)(reshaped_trk_features)

    # Normalize the input features
    normed_trk_features = keras.layers.BatchNormalization()(masked_trk_features)

    # Embed features in latent space with embedding_dims
    encoded_trk_features = keras.layers.Dense(
        units=embedding_dims,
        activation=keras.activations.selu,
        name="Embedding_trk_dense_1")(normed_trk_features)
    encoded_trk_features = keras.layers.Dropout(dropout_rate, name="Embedding_trk_dropout_1")(encoded_trk_features)
    encoded_trk_features = keras.layers.BatchNormalization(name="Embedding_trk_batchnorm")(encoded_trk_features)
    encoded_trk_features = keras.layers.Dense(
        units=embedding_dims,
        activation=keras.activations.selu,
        name="Embedding_trk_dense_2")(encoded_trk_features)
    encoded_trk_features = keras.layers.Dropout(dropout_rate, name="Embedding_trk_dropout_2")(encoded_trk_features)

    # Preprocess the ecl features

    # Get the ecl features
    raw_ecl_features = raw_features[:, ecl_start:ecl_start+num_ecl*num_ecl_features]

    # 3D tensor with axes for samples, ecl candidates and features
    reshaped_ecl_features = keras.layers.Reshape((num_ecl, num_ecl_features))(raw_ecl_features)

    # Create a keras mask for padded ecl candidates
    masked_ecl_features = keras.layers.Masking(mask_value=mask_value)(reshaped_ecl_features)

    # Normalize the input features
    normed_ecl_features = keras.layers.BatchNormalization()(masked_ecl_features)

    # Embed features in latent space with embedding_dims
    encoded_ecl_features = keras.layers.Dense(
        units=embedding_dims,
        activation=keras.activations.selu,
        name="Embedding_ecl_dense_1")(normed_ecl_features)
    encoded_ecl_features = keras.layers.Dropout(dropout_rate, name="Embedding_ecl_dropout_1")(encoded_ecl_features)
    encoded_ecl_features = keras.layers.BatchNormalization(name="Embedding_ecl_batchnorm")(encoded_ecl_features)
    encoded_ecl_features = keras.layers.Dense(
        units=embedding_dims,
        activation=keras.activations.selu,
        name="Embedding_ecl_dense_2")(encoded_ecl_features)
    encoded_ecl_features = keras.layers.Dropout(dropout_rate, name="Embedding_ecl_dropout_2")(encoded_ecl_features)

    # Preprocess the rest of event features

    # Get the roe features
    raw_roe_features = raw_features[:, roe_start:roe_start+num_roe*num_roe_features]

    # 3D tensor with axes for samples, roe's and features
    reshaped_roe_features = keras.layers.Reshape((num_roe, num_roe_features))(raw_roe_features)

    # Create a keras mask for padded roe
    masked_roe_features = keras.layers.Masking(mask_value=mask_value)(reshaped_roe_features)

    # Normalize the input features
    normed_roe_features = keras.layers.BatchNormalization()(masked_roe_features)

    # Embed features in latent space with embedding_dims
    encoded_roe_features = keras.layers.Dense(
        units=embedding_dims,
        activation=keras.activations.selu,
        name="Embedding_roe_dense_1")(normed_roe_features)
    encoded_roe_features = keras.layers.Dropout(dropout_rate, name="Embedding_roe_dropout_1")(encoded_roe_features)
    encoded_roe_features = keras.layers.BatchNormalization(name="Embedding_roe_batchnorm")(encoded_roe_features)
    encoded_roe_features = keras.layers.Dense(
        units=embedding_dims,
        activation=keras.activations.selu,
        name="Embedding_roe_dense_2")(encoded_roe_features)
    encoded_roe_features = keras.layers.Dropout(dropout_rate, name="Embedding_roe_dropout_2")(encoded_roe_features)

    # Concatenate all encoded features and their masks
    encoded_features = MyConcatenate()([encoded_trk_features, encoded_ecl_features, encoded_roe_features])

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
