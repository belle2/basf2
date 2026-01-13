#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import glob
import numpy as np
import pyarrow.parquet as pq
import pyarrow as pa
import pandas as pd
import argparse
from tflat.config import config


def merge_parquets(input_dir, output_dir, mask_value, tree_name="tflat_variables"):
    '''
    Merges sampled parquet files. Rescales target variable to be >=0 and masks NaN values.
    '''
    files = sorted(glob.glob(os.path.join(input_dir, "standard_tflat_training_data*.parquet")))
    schema = pq.ParquetFile(files[0]).schema_arrow
    writer = pq.ParquetWriter(output_dir+'tflat_samples_merged.parquet', schema=schema, compression='snappy')
    print("Merging parquet files:")
    for i in range(len(files)):
        print(f"\r{i+1}/{len(files)}", end="", flush=True)
        f = files[i]
        table = pq.read_table(f, schema=schema)
        df = table.to_pandas()

        # Rescale target variable from [-1,1] to [0,1]
        m = df["qrCombined"].min()
        if m != 0:
            df["qrCombined"] = df["qrCombined"].where(df["qrCombined"] != m, 0)

        # verify two-class output
        assert len(df["qrCombined"].unique()) == 2

        # Mask NaN values
        df = df.fillna(mask_value)
        # Remove columns containing meta variables
        for column in df.columns:
            if (column[0:2] == '__') & (column[-2:] == '__'):
                df = df.drop(column, axis=1)

        table = pa.Table.from_pandas(df)
        writer.write_table(table)

    writer.close()


def create_dataset(pf, parquet_path, index, chunk_size, n_rowgroups, rowgroup_edges):
    '''
    Picks rows from parquet file according to the given index array.
    Created parquet file is segmented into rowgroups with maximum size given by chunk_size.
    '''
    writer = None
    n_chunks = len(index)//chunk_size + 1
    for chunk in range(n_chunks):
        print(f"\r{chunk+1}/{n_chunks}", end="", flush=True)
        chunk_df = pd.DataFrame()
        # Handle special case of last chunk having fewer rows
        if (chunk+1)*chunk_size > len(index):
            index_chunks = index[chunk*chunk_size:]
        else:
            index_chunks = index[chunk*chunk_size:(chunk+1)*chunk_size]
        lower_edge = 0
        for rowgroup in range(n_rowgroups):
            upper_edge = rowgroup_edges[rowgroup]
            # Select rows from rowgroup that belong into current chunk
            rows_to_fetch = index_chunks[(index_chunks >= lower_edge) & (index_chunks < upper_edge)]
            if len(rows_to_fetch) > 0:
                table = pf.read_row_group(rowgroup)
                df = table.to_pandas()
                df = df.iloc[(rows_to_fetch-lower_edge)]
                chunk_df = pd.concat([chunk_df, df])
            lower_edge = upper_edge
        table = pa.Table.from_pandas(chunk_df)
        if writer is None:
            writer = pq.ParquetWriter(parquet_path, table.schema, compression='NONE')
        writer.write_table(table)
    writer.close()


def shuffle_and_chunk_parquet(output_dir, val_split, chunk_size):
    '''
    Splits single parquet file into a training and validation parquet file.
    The data contained in the resulting files is shuffled and segmented into chunks.
    '''
    pf = pq.ParquetFile(output_dir+'tflat_samples_merged.parquet')
    rowgroup_edges = []
    n_rows = 0
    n_rowgroups = pf.num_row_groups
    for i in range(n_rowgroups):
        n = pf.metadata.row_group(i).num_rows
        n_rows += n
        rowgroup_edges.append(n_rows)
    index = np.arange(n_rows)
    np.random.shuffle(index)
    n_training_samples = int(n_rows*val_split)
    index_training = index[:n_training_samples]
    index_validation = index[n_training_samples:]
    print('\nCreating training dataset:')
    create_dataset(pf, output_dir+'tflat_training_samples.parquet', index_training, chunk_size, n_rowgroups, rowgroup_edges)
    print('\nCreating validation dataset:')
    create_dataset(pf, output_dir+'tflat_validation_samples.parquet', index_validation, chunk_size, n_rowgroups, rowgroup_edges)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Train TFlat')
    parser.add_argument(
        '--parquet_in',
        dest='input_dir',
        type=str,
        help='Path to directory where sampled parquet files are stored'
    )
    parser.add_argument(
        '--parquet_out',
        dest='output_dir',
        type=str,
        help='Path to directory where training and validation parquet files are saved to'
    )
    args = parser.parse_args()
    input_dir = args.input_dir
    output_dir = args.output_dir
    os.makedirs(output_dir, exist_ok=True)
    val_split = config['train_valid_fraction']
    chunk_size = config['chunk_size']
    mask_value = config['parameters']['mask_value']

    merge_parquets(
        input_dir=input_dir,
        output_dir=output_dir,
        mask_value=mask_value,
    )
    shuffle_and_chunk_parquet(output_dir, val_split, chunk_size)
    print("\nDone!")
