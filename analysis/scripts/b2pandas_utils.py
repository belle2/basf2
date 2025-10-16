##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from typing import List, Optional
import basf2
import variables
import tables
import numpy as np
import warnings
from pyarrow.parquet import ParquetWriter
from pyarrow.csv import CSVWriter
from pyarrow import ipc
import pyarrow as pa


"""
Python utilities to help create or manage ntuples and work with them in pandas
"""

numpy_to_pyarrow_type_map = {
    np.int32: pa.int32(),
    np.int64: pa.int64(),
    np.uint32: pa.uint32(),
    np.uint64: pa.uint64(),
    np.float32: pa.float32(),
    np.float64: pa.float64(),
    np.bool_: pa.bool_(),
    np.object_: pa.string(),
    np.str_: pa.string(),
}


class VariablesToTable(basf2.Module):
    """
    Base class to dump ntuples into a non root format of your choosing
    """

    def __init__(
        self,
        listname: str,
        variables: List[str],
        filename: str,
        hdf_table_name: Optional[str] = None,
        event_buffer_size: int = 100,
        **writer_kwargs,
    ):
        """Constructor to initialize the internal state

        Arguments:
            listname(str): name of the particle list
            variables(list(str)): list of variables to save for each particle
            filename(str): name of the output file to be created.
                Needs to end with `.csv` for csv output, `.parquet` or `.pq` for parquet output,
                `.h5`, `.hdf` or `.hdf5` for hdf5 output and `.feather` or `.arrow` for feather output
            hdf_table_name(str): name of the table in the hdf5 file.
                If not provided, it will be the same as the listname
            event_buffer_size(int): number of events to buffer before writing to disk,
                higher values will use more memory but write faster and result in smaller files
            **writer_kwargs: additional keyword arguments to pass to the writer.
                For details, see the documentation of the writer in the apache arrow documentation.
                Only use, if you know what you are doing!
        """
        super().__init__()
        #: Output filename
        self._filename = filename
        #: Particle list name
        self._listname = listname
        #: List of variables
        self._variables = list(set(variables))
        #: File type
        file_type = self._filename.split(".")[-1]
        #: Output format
        if file_type in ["csv"]:
            self._format = "csv"
        elif file_type in ["parquet", "pq"]:
            self._format = "parquet"
        elif file_type in ["h5", "hdf", "hdf5"]:
            self._format = "hdf5"
        elif file_type in ["feather", "arrow"]:
            self._format = "feather"
        else:
            raise ValueError(
                f"Unknown file type ending .{file_type}, supported types are 'csv', "
                "'parquet', 'pq', 'h5', 'hdf', 'hdf5', 'feather' or 'arrow'"
            )
        #: Table name in the hdf5 file
        self._table_name = (
            hdf_table_name if hdf_table_name is not None else self._listname
        )
        #: Event buffer size
        self._event_buffer_size = event_buffer_size
        #: Event buffer counter
        self._event_buffer_counter = 0
        #: writer kwargs
        self._writer_kwargs = writer_kwargs

    def initialize(self):
        """
        Setup variable lists, pointers, buffers and file writers
        """
        # Always avoid the top-level 'import ROOT'.
        import ROOT  # noqa

        #: variable names
        self._varnames = [
            str(varname)
            for varname in variables.variables.resolveCollections(
                variables.std_vector(*self._variables)
            )
        ]

        #: std::vector of variable names
        self._std_varnames = variables.std_vector(*self._varnames)

        #: Event metadata
        self._evtmeta = ROOT.Belle2.PyStoreObj("EventMetaData")
        self._evtmeta.isRequired()

        #: Pointer to the particle list
        self._plist = ROOT.Belle2.PyStoreObj(self._listname)
        self._plist.isRequired()

        dtypes = [
            ("__experiment__", np.int32),
            ("__run__", np.int32),
            ("__event__", np.uint32),
            ("__production__", np.uint32),
            ("__candidate__", np.uint32),
            ("__ncandidates__", np.uint32),
        ]
        for name in self._varnames:
            # only float variables for now
            dtypes.append((name, np.float64))

        #: The data type
        self._dtypes = dtypes

        #: event variables buffer (will be automatically grown if necessary)
        self._buffer = np.empty(self._event_buffer_size * 10, dtype=self._dtypes)

        #: current start index in the event variables buffer
        self._buffer_index = 0

        if self._format == "hdf5":
            self.initialize_hdf5_writer()
        elif self._format == "parquet":
            self.initialize_parquet_writer()
        elif self._format == "csv":
            self.initialize_csv_writer()
        elif self._format == "feather":
            self.initialize_feather_writer()

    @property
    def buffer(self):
        """
        The buffer slice across multiple entries
        """
        return self._buffer[:self._buffer_index]

    @property
    def event_buffer(self):
        """
        The buffer slice for the current event
        """
        return self._buffer[self._buffer_index - self._plist.getListSize(): self._buffer_index]

    def clear_buffer(self):
        """
        Reset the buffer event counter and index
        """
        self._event_buffer_counter = 0
        self._buffer_index = 0

    def append_buffer(self):
        """
        "Append" a new event to the buffer by moving the buffer index forward by particle list size

        Automatically replaces the buffer by a larger one if necessary
        """
        plist_size = self._plist.getListSize()
        if (plist_size + self._buffer_index) > len(self._buffer):
            new_buffer = np.empty(
                # factor 1.5 larger or at least as large as necessary
                max(int(len(self._buffer) * 1.5), self._buffer_index + plist_size),
                dtype=self._dtypes,
            )
            new_buffer[:self._buffer_index] = self.buffer
            self._buffer = new_buffer
        self._buffer_index += plist_size
        self._event_buffer_counter += 1

    def initialize_feather_writer(self):
        """
        Initialize the feather writer using pyarrow
        """
        #: A list of tuples and py.DataTypes to define the pyarrow schema
        self._schema = [
            (name, numpy_to_pyarrow_type_map[dt]) for name, dt in self._dtypes
        ]
        #: a writer object to write data into a feather file
        self._feather_writer = ipc.RecordBatchFileWriter(
            sink=self._filename,
            schema=pa.schema(self._schema),
            **self._writer_kwargs,
        )

    def initialize_parquet_writer(self):
        """
        Initialize the parquet writer using pyarrow
        """
        #: A list of tuples and py.DataTypes to define the pyarrow schema
        self._schema = [
            (name, numpy_to_pyarrow_type_map[dt]) for name, dt in self._dtypes
        ]
        #: a writer object to write data into a parquet file
        self._parquet_writer = ParquetWriter(
            self._filename, schema=pa.schema(self._schema), **self._writer_kwargs
        )

    def initialize_csv_writer(self):
        """
        Initialize the csv writer using pyarrow
        """
        #: A list of tuples and py.DataTypes to define the pyarrow schema
        self._schema = [
            (name, numpy_to_pyarrow_type_map[dt]) for name, dt in self._dtypes
        ]
        #: a writer object to write data into a csv file
        self._csv_writer = CSVWriter(self._filename, schema=pa.schema(self._schema), **self._writer_kwargs)

    def initialize_hdf5_writer(self):
        """
        Initialize the hdf5 writer using pytables
        """
        #: The pytable file
        self._hdf5_writer = tables.open_file(
            self._filename, mode="w", title="Belle2 Variables to HDF5"
        )
        filters = tables.Filters(complevel=1, complib="blosc:lz4", fletcher32=False)

        # some variable names are not just A-Za-z0-9 so pytables complains but
        # seems to work. Ignore warning
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            #: The pytable
            self._table = self._hdf5_writer.create_table(
                "/", self._table_name, obj=np.zeros(0, self._dtypes), filters=filters, **self._writer_kwargs
            )

    def fill_event_buffer(self):
        """
        Assign values for all variables for all particles in the particle list to the current event buffer
        """
        buf = self.event_buffer

        # add some extra columns for bookkeeping
        buf["__experiment__"] = self._evtmeta.getExperiment()
        buf["__run__"] = self._evtmeta.getRun()
        buf["__event__"] = self._evtmeta.getEvent()
        buf["__production__"] = self._evtmeta.getProduction()
        buf["__ncandidates__"] = len(buf)
        buf["__candidate__"] = np.arange(len(buf))

        # fill variables into buffer
        vector = variables.variables.evaluateVariables(self._std_varnames, self._plist)
        values = np.array(vector.data()).reshape(-1, len(self._varnames))
        for name, col in zip(self._varnames, values.T):
            buf[name] = col

    @property
    def buffer_full(self):
        """
        check if the buffer is full
        """
        return self._event_buffer_counter == self._event_buffer_size

    def write_buffer(self):
        """
        write the buffer to the output file
        """
        if self._format == "hdf5":
            """Create a new row in the hdf5 file with for each particle in the list"""
            # \cond false positive doxygen warning
            self._table.append(self.buffer)
            # \endcond
        else:
            table = {name: self.buffer[name] for name, _ in self._dtypes}
            pa_table = pa.table(table, schema=pa.schema(self._schema))
            if self._format == "parquet":
                self._parquet_writer.write_table(pa_table)
            elif self._format == "csv":
                self._csv_writer.write(pa_table)
            elif self._format == "feather":
                self._feather_writer.write_table(pa_table)

    def event(self):
        """
        Event processing function

        executes the fill_buffer function and writes the data to the output file
        in chunks of event_buffer_size
        """
        self.append_buffer()
        self.fill_event_buffer()
        # \cond false positive doxygen warning
        if self.buffer_full:
            self.write_buffer()
            self.clear_buffer()
        # \endcond

    def terminate(self):
        """save and close the output"""
        import ROOT  # noqa
        # \cond false positive doxygen warning
        if len(self.buffer) > 0:
            self.write_buffer()
        # \endcond

        if self._format == "hdf5":
            self._table.flush()
            self._hdf5_writer.close()
        elif self._format == "parquet":
            self._parquet_writer.close()
        elif self._format == "csv":
            self._csv_writer.close()
        elif self._format == "feather":
            self._feather_writer.close()
        ROOT.Belle2.MetadataService.Instance().addNtuple(self._filename)


class VariablesToHDF5(VariablesToTable):
    """
    Legacy class to not break existing code
    """

    def __init__(self, listname, variables, filename, hdf_table_name: Optional[str] = None):
        """
        Constructor for the legacy HDF5 writer.
        """
        super().__init__(listname, variables, filename, hdf_table_name)
        assert self._filename.split(".")[-1] in ["h5", "hdf", "hdf5"], (
            "Filename must end with .h5, .hdf or .hdf5 for HDF5 output. "
            f"Got {self._filename}"
        )


def make_mcerrors_readable(dataframe, column="mcErrors"):
    """
    Take a dataframe containing a column with the output of the :b2:var:`mcErrors`
    variable from :b2:mod:`VariablesToNTuple` and convert it to a readable set
    of columns of the form ``{column}_{name}`` where column is the value of the
    ``column`` argument and ``name`` is one of the :ref:`mcmatching`
    error flags (without the leading 'c_').

    Arguments:
        dataframe(pandas.DataFrame): the pandas dataframe containing an ntuple
                with column containing the output of the  mcErrors variable
        column(str): the name containing the values from the mcErrors variable
    """
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa

    if column not in dataframe:
        raise KeyError(f"Cannot find column '{column}'")

    # convert mcErrors to int to be able to logical operate on it
    mcErrors = dataframe[column].astype(int)

    # and loop over all the c_ constants in the Belle2.MCMatching class
    for flag in (e for e in dir(ROOT.Belle2.MCMatching) if e.startswith("c_")):
        try:
            value = int(getattr(ROOT.Belle2.MCMatching, flag))
        except ValueError:
            # probably the extraInfo column name, ignore
            continue

        # and set the column
        name = column + flag[1:]
        if value == 0:
            dataframe[name] = mcErrors == 0
        else:
            dataframe[name] = (mcErrors & value) == value
