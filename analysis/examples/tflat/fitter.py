#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import keras
import ROOT
import threading
import pyarrow.parquet as pq
import time


class batch_generator(keras.utils.PyDataset):
    '''
    Generator that reads the input data into memory in chunks.
    '''

    def __init__(self, parquet_path, variables, target, batch_size, chunk_size):
        """
        Prepare all variables and prefetch 2 chunks.
        """
        super().__init__(workers=1, use_multiprocessing=False, max_queue_size=10)
        #: List of input variable names
        self.variables = variables
        #: Name of target variable
        self.target = target
        #: Batch size of the model
        self.batch_size = batch_size
        #: Parquet metadata
        self.pf = pq.ParquetFile(parquet_path)
        #: Number of batches in a chunk
        self.max_batches = chunk_size//batch_size
        #: Number of chunks in the data file
        self.n_chunks = self.pf.num_row_groups
        #: Index of chunk currently in use
        self.current_chunk_idx = 0
        #: Number of rows in datafile
        self.dataset_length = sum(
            self.pf.metadata.row_group(i).num_rows for i in range(self.n_chunks)
        )

        # Multithreading
        #: Chunklock to avoid race conditions
        self.chunk_lock = threading.Lock()
        #: Flag that indicated weather the new chunk is done loading into memory
        self.chunk_ready = False
        #: Thread that loads new chunk while main thread is training
        self.loader_thread = None

        # Prefetch first chunk
        self._start_async_load()
        self._wait_for_chunk()  # ensure first chunk is ready
        #: Chunk currently in use
        self.chunk_in_use = self.chunk_in_waiting

        # Prepare next chunk
        self._start_async_load()

        #: Index of current batch in current chunk
        self.current_batch = 0

    def __len__(self):
        """
        Returns number of batches in dataset
        """
        return self.dataset_length // self.batch_size

    def __getitem__(self, idx):
        """
        Returns the next batch used in training
        """
        if self.current_batch >= self.max_batches:
            self._wait_for_chunk()

            with self.chunk_lock:
                self.chunk_in_use = self.chunk_in_waiting
                self.max_batches = self.max_batches_next
                self.current_batch = 0

            self._start_async_load()

        X, y = self._get_batch(self.current_batch)
        self.current_batch += 1
        return X, y

    def _load_chunk(self):
        """
        Load next chunk from datafile and shuffle it
        """
        rg = self.current_chunk_idx
        table = self.pf.read_row_group(rg)
        # Shuffle data
        df = table.to_pandas().sample(frac=1).reset_index(drop=True)

        X = df[self.variables].to_numpy()
        y = df[self.target].to_numpy()
        max_batches = len(df) // self.batch_size

        # Publish chunk
        with self.chunk_lock:
            #: Next chunk
            self.chunk_in_waiting = (X, y)
            #: Maximum number of batches in this chunk
            self.max_batches_next = max_batches
            self.chunk_ready = True

        # Move to next row group
        self.current_chunk_idx = (self.current_chunk_idx + 1) % self.n_chunks

    def _start_async_load(self):
        '''
        Start new thread to load new chunk
        '''
        self.chunk_ready = False
        self.loader_thread = threading.Thread(target=self._load_chunk, daemon=True)
        self.loader_thread.start()

    def _wait_for_chunk(self):
        '''
        Sleep until second thread is finished with loading the next chunk
        '''
        while not self.chunk_ready:
            time.sleep(5)

    def _get_batch(self, batch_idx):
        '''
        Extract next batch from chunk
        '''
        X, y = self.chunk_in_use
        i0 = batch_idx * self.batch_size
        i1 = i0 + self.batch_size
        return X[i0:i1], y[i0:i1]


def fit(model, train_file, val_file, treename, variables, target_variable, config, checkpoint_filepath):
    variables = list(map(ROOT.Belle2.MakeROOTCompatible.makeROOTCompatible, variables))
    batch_size = config['batch_size']
    chunk_size = config['chunk_size']

    train_ds = batch_generator(train_file, variables, target_variable, batch_size, chunk_size)
    val_ds = batch_generator(val_file, variables, target_variable, batch_size, chunk_size)

    # configure early stopping callback
    callbacks = [keras.callbacks.EarlyStopping(
        monitor='val_loss',
        min_delta=0,
        patience=config['patience'],
        verbose=1,
        mode='auto',
        baseline=None,
        restore_best_weights=True)]

    # configure checkpointing callback
    model_checkpoint_callback = keras.callbacks.ModelCheckpoint(
        filepath=checkpoint_filepath,
        monitor='val_loss',
        mode='min',
        save_best_only=True)
    callbacks.append(model_checkpoint_callback)

    # perform fit() with callbacks
    model.fit(
        train_ds,
        validation_data=val_ds,
        steps_per_epoch=len(train_ds),
        validation_steps=len(val_ds),
        epochs=config['epochs'],
        callbacks=callbacks,
        verbose=2)

    print()
