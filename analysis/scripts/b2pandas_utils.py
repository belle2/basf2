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
        event_buffer_size: int = 10,
    ):
        """Constructor to initialize the internal state

        Arguments:
            listname(str): name of the particle list
            variables(list(str)): list of variables to save for each particle
            filename(str): name of the output file to be created.
                Needs to end with `.csv` for csv output, `.parquet` or `.pq` for parquet output,
                and `.h5`, `.hdf` or `.hdf5` for hdf5 output
            hdf_table_name(str): name of the table in the hdf5 file.
                If not provided, it will be the same as the listname
            event_buffer_size(int): number of events to buffer before writing to disk,
                higher values will use more memory but write faster and result in smaller files
        """
        super().__init__()
        #: Output filename
        self._filename = filename
        #: Particle list name
        self._listname = listname
        #: List of variables
        self._variables = list(set(variables))
        #: Output format
        file_type = self._filename.split(".")[-1]
        if file_type in ["csv"]:
            self._format = "csv"
        elif file_type in ["parquet", "pq"]:
            self._format = "parquet"
        elif file_type in ["h5", "hdf", "hdf5"]:
            self._format = "hdf5"
        else:
            raise ValueError(
                f"Unknown file type ending .{file_type}, supported types are 'csv', 'parquet', 'hdf5'."
            )
        #: Table name in the hdf5 file
        self._table_name = (
            hdf_table_name if hdf_table_name is not None else self._listname
        )
        #: Event buffer size
        self._event_buffer_size = event_buffer_size
        #: Event buffer counter
        self._event_buffer_counter = 0

    def initialize(self):
        """Create the hdf5 file and list of variable objects to be used during
        event processing."""
        # Always avoid the top-level 'import ROOT'.
        import ROOT  # noqa

        #: variable names
        self._varnames = [
            str(varname)
            for varname in variables.variables.resolveCollections(
                variables.std_vector(*self._variables)
            )
        ]
        #: variable objects for each variable
        self._var_objects = [variables.variables.getVariable(n) for n in self._varnames]

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

        if self._format == "hdf5":
            self.initialize_hdf5_writer()
        elif self._format == "parquet":
            self.initialize_parquet_writer()
        elif self._format == "csv":
            self.initialize_csv_writer()
        else:
            raise ValueError(
                f"Unknown format {self._format}, supported formats are 'hdf5', 'parquet', 'csv'."
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
            self._filename, schema=pa.schema(self._schema)
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
        self._csv_writer = CSVWriter(self._filename, schema=pa.schema(self._schema))

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
                "/", self._table_name, obj=np.zeros(0, self._dtypes), filters=filters
            )

    def fill_event_buffer(self):
        """
        collect all variables for the particle in a numpy array
        """

        # create a numpy array with the data
        buf = np.empty(self._plist.getListSize(), dtype=self._dtypes)
        # add some extra columns for bookkeeping
        buf["__experiment__"] = self._evtmeta.getExperiment()
        buf["__run__"] = self._evtmeta.getRun()
        buf["__event__"] = self._evtmeta.getEvent()
        buf["__production__"] = self._evtmeta.getProduction()
        buf["__ncandidates__"] = len(buf)
        buf["__candidate__"] = np.arange(len(buf))

        for row, p in zip(buf, self._plist):
            for name, v in zip(self._varnames, self._var_objects):
                # pyroot proxy not working with callables, we should fix this.
                # For now we need to go back by name and call it.
                # should be `row[v.name] = v.func(p)`
                row[name] = variables.variables.evaluate(v.name, p)
        return buf

    def fill_buffer(self):
        """
        fill a buffer ofer multiple events and return it, when self.
        """
        if self._event_buffer_counter == 0:
            self._buffer = self.fill_event_buffer()
        else:
            self._buffer = np.concatenate((self._buffer, self.fill_event_buffer()))

        self._event_buffer_counter += 1
        if self._event_buffer_counter == self._event_buffer_size:
            self._event_buffer_counter = 0
            return self._buffer
        return None

    def event(self):
        """
        Event processing function
        executes the fill_buffer function and writes the data to the output file
        """
        buf = self.fill_buffer()
        if buf is None:
            return
        if self._format == "hdf5":
            """Create a new row in the hdf5 file with for each particle in the list"""
            self._table.append(buf)
        elif self._format == "parquet":
            table = {name: buf[name] for name, _ in self._dtypes}
            pa_table = pa.table(table, schema=pa.schema(self._schema))
            self._parquet_writer.write_table(pa_table)
        elif self._format == "csv":
            table = {name: buf[name] for name, _ in self._dtypes}
            pa_table = pa.table(table, schema=pa.schema(self._schema))
            self._csv_writer.write(pa_table)

    def terminate(self):
        """save and close the output"""
        import ROOT  # noqa

        if self._format == "hdf5":
            self._table.flush()
            self._hdf5_writer.close()
        elif self._format == "parquet":
            self._parquet_writer.close()
        elif self._format == "csv":
            self._csv_writer.close()
        ROOT.Belle2.MetadataService.Instance().addNtuple(self._filename)


class VariablesToHDF5(VariablesToTable):
    """
    Legacy class to not break existing code
    """

    def __init__(self, listname, variables, filename):
        super().__init__(listname, variables, filename, "hdf5")


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
