##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from typing import List
import basf2
import variables
import tables
import numpy as np
import warnings
import pyarrow.parquet as parquet
import pyarrow as pa
from pyarrow import csv


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


class VariablesToNotRoot(basf2.Module):
    """
    Base class to dump ntuples into a non root format of your choosing
    """

    def __init__(self, listname: str, variables: List[str], filename: str, format: str):
        """Constructor to initialize the internal state

        Arguments:
            listname(str): name of the particle list
            variables(list(str)): list of variables to save for each particle
            filename(str): name of the hdf5 file to be created
            format(str): format of the output file, one of 'hdf5', 'parquet', 'csv'
        """
        super().__init__()
        #: Output filename
        self._filename = filename
        #: Particle list name
        self._listname = listname
        #: List of variables
        self._variables = variables
        #: Output format
        self._format = format

    def initialize(self):
        """Create the hdf5 file and list of variable objects to be used during
        event processing."""
        # Always avoid the top-level 'import ROOT'.
        import ROOT  # noqa
        #: variable names
        self._varnames = [
            str(varname) for varname in variables.variables.resolveCollections(
                variables.std_vector(
                    *self._variables))]
        #: variable objects for each variable
        self._var_objects = [variables.variables.getVariable(n) for n in self._varnames]

        #: Event metadata
        self._evtmeta = ROOT.Belle2.PyStoreObj("EventMetaData")
        self._evtmeta.isRequired()
        #: Pointer to the particle list
        self._plist = ROOT.Belle2.PyStoreObj(self._listname)
        self._plist.isRequired()

        dtypes = [
            ("exp", np.int32), ("run", np.int32), ("evt", np.uint32),
            ("prod", np.uint32), ("icand", np.uint32), ("ncand", np.uint32)
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
            raise ValueError(f"Unknown format {self._format}")

    def initialize_parquet_writer(self):
        """
        Initialize the parquet writer using pyarrow
        """
        self._schema = [(name, numpy_to_pyarrow_type_map[dt]) for name, dt in self._dtypes]
        self._parquet_writer = parquet.ParquetWriter(self._filename, schema=pa.schema(self._schema))

    def initialize_csv_writer(self):
        """
        Initialize the csv writer using pyarrow
        """
        self._schema = [(name, numpy_to_pyarrow_type_map[dt]) for name, dt in self._dtypes]
        self._csv_writer = csv.CSVWriter(self._filename, schema=pa.schema(self._schema))

    def initialize_hdf5_writer(self):
        """
        Initialize the hdf5 writer using pytables
        """

        self._hdf5_writer = tables.open_file(self._filename, mode="w", title="Belle2 Variables to HDF5")
        filters = tables.Filters(complevel=1, complib='blosc:lz4', fletcher32=False)

        # some variable names are not just A-Za-z0-9 so pytables complains but
        # seems to work. Ignore warning
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            #: The pytable
            self._table = self._hdf5_writer.create_table("/", self._listname, obj=np.zeros(0, self._dtypes), filters=filters)

    def fill_buffer(self):
        """
        collect all variables for the particle in a numpy array
        """

        # create a numpy array with the data
        buf = np.empty(self._plist.getListSize(), dtype=self._dtypes)
        # add some extra columns for bookkeeping
        buf["exp"] = self._evtmeta.getExperiment()
        buf["run"] = self._evtmeta.getRun()
        buf["evt"] = self._evtmeta.getEvent()
        buf["prod"] = self._evtmeta.getProduction()
        buf["ncand"] = len(buf)
        buf["icand"] = np.arange(len(buf))

        for row, p in zip(buf, self._plist):
            for name, v in zip(self._varnames, self._var_objects):
                # pyroot proxy not working with callables, we should fix this.
                # For now we need to go back by name and call it.
                # should be `row[v.name] = v.func(p)`
                row[name] = variables.variables.evaluate(v.name, p)
        return buf

    def event(self):
        """
        Event processing function
        executes the fill_buffer function and writes the data to the output file
        """
        buf = self.fill_buffer()

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
        if self._format == "hdf5":
            self._table.flush()
            self._hdf5_writer.close()
            import ROOT
            ROOT.Belle2.MetadataService.Instance().addHDF5File(self._filename)
        elif self._format == "parquet":
            self._parquet_writer.close()
        elif self._format == "csv":
            self._csv_writer.close()


class VariablesToHDF5(VariablesToNotRoot):
    """
    Legacy class to not break existing code
    """

    def __init__(self, listname, variables, filename):
        super().__init__(listname, variables, filename, "hdf5")


def make_mcerrors_readable(dataframe, column="mcErrors"):
    """
    Take a dataframe containing an column with the output of the :b2:var:`mcErrors`
    variable from :b2:mod:`VariablesToNTuple` and convert it to a readable set
    of columns of the form ``{column}_{name}`` where column is the value of the
    ``column`` argument and ``name`` is one of one of the :ref:`mcmatching`
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


# This is just for testing, no need for doxygen to weirdly document it
# @cond
if __name__ == "__main__":
    import modularAnalysis

    p = basf2.create_path()
    p.add_module("EventInfoSetter", evtNumList=1000)
    p.add_module("EvtGenInput")
    modularAnalysis.fillParticleListsFromMC([("pi-:gen", "")], path=p)
    a = VariablesToNotRoot("pi-:gen", ["M", "E", "px", "py", "pz"], "test.parquet", "parquet")
    p.add_module(a)
    # Process the events
    basf2.process(p)
    print(basf2.statistics)
# @endcond
