import basf2
from ROOT import Belle2
import tables
import numpy as np
import warnings
from variables import variables as variable_manager
from variables import std_vector

"""
Python uilities to help create or manage ntuples and work with them in pandas
"""


class VariablesToHDF5(basf2.Module):
    """
    Dump variables directly to HDF5

    This Module is the equivalent of VariablesToNtuple but creates an hdf5 file
    instead of a root file. It is slower as it is implemented in pure python and
    should currently be considered a proof of concept.
    """

    def __init__(self, listname, variables, filename):
        """Constructor to initialize the internal state

        Arguments:
            listname(str): name of the particle list
            variables(list(str)): list of variables to save for each particle
            filename(str): name of the hdf5 file to be created
        """
        super().__init__()
        #: Output filename
        self._filename = filename
        #: Particle list name
        self._listname = listname
        #: List of variables
        self._variables = variables

    def initialize(self):
        """Create the hdf5 file and list of variable objects to be used during
        event processing."""
        #: variable names
        self._varnames = [str(varname) for varname in variable_manager.resolveCollections(std_vector(*self._variables))]
        #: variable objects for each variable
        self._var_objects = [variable_manager.getVariable(n) for n in self._varnames]

        #: Event metadata
        self._evtmeta = Belle2.PyStoreObj("EventMetaData")
        self._evtmeta.isRequired()
        #: Pointer to the particle list
        self._plist = Belle2.PyStoreObj(self._listname)
        self._plist.isRequired()

        #: The hdf5 file
        self._hdf5file = tables.open_file(self._filename, mode="w", title="Belle2 Variables to HDF5")
        if not self._hdf5file:
            basf2.B2ERROR("Cannot create output file")
            return

        dtype = [("exp", np.int32), ("run", np.int32), ("evt", np.uint32),
                 ("prod", np.uint32), ("icand", np.uint32), ("ncand", np.uint32)]
        for name in self._varnames:
            # only float variables for now
            dtype.append((name, np.float64))

        #: The data type
        self._dtype = dtype
        filters = tables.Filters(complevel=1, complib='blosc:lz4', fletcher32=False)
        # some variable names are not just A-Za-z0-9 so pytables complains but
        # seems to work. Ignore warning
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            #: The pytable
            self._table = self._hdf5file.create_table("/", self._listname, obj=np.zeros(0, dtype), filters=filters)

    def event(self):
        """Create a new row in the hdf5 file with for each particle in the list"""
        buf = np.empty(self._plist.getListSize(), dtype=self._dtype)
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
                row[name] = variable_manager.evaluate(v.name, p)

        self._table.append(buf)

    def terminate(self):
        """save and close the output"""
        self._table.flush()
        self._hdf5file.close()


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

    if column not in dataframe:
        raise KeyError(f"Cannot find coulumn '{column}'")

    # convert mcErrors to int to be able to logical operate on it
    mcErrors = dataframe[column].astype(int)

    # and loop over all the c_ constants in the Belle2.MCMatching class
    for flag in (e for e in dir(Belle2.MCMatching) if e.startswith("c_")):
        try:
            value = int(getattr(Belle2.MCMatching, flag))
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
    p.add_module("EventInfoSetter", evtNumList=100)
    p.add_module("EvtGenInput")
    modularAnalysis.fillParticleListsFromMC([("pi-:gen", "")], path=p)
    a = VariablesToHDF5("pi-:gen", ["M", "E", "px", "py", "pz"], "test.hdf5")
    p.add_module(a)
    # Process the events
    basf2.process(p)
    print(basf2.statistics)
# @endcond
