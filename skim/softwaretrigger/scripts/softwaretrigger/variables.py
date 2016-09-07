import basf2
from ROOT import Belle2

from root_pandas import to_root


class SummarizeTriggerVariables(basf2.Module):
    """
    Small helper module to export the software trigger variables saved in the data store to
    a ROOT file for later analysis.
    """

    def __init__(self, root_file_name="save_vars.root", store_object_name="SoftwareTriggerVariable"):
        """
        Create a new SummarizeTriggerVariables module.
        :param root_file_name: The root file name to save to results to.
        :param store_object_name: The name of the object in the data store from where to get the variables.
        """
        basf2.Module.__init__(self)

        self.df = pd.DataFrame()
        self.root_file_name = root_file_name
        self.store_object_name = store_object_name

        self.event_number_counter = 0

    def event(self):
        """
        Extract the variables from the data store and save them in an internal pandas data frame.
        Also increment the internal event number counter.
        """
        variables = Belle2.PyStoreObj(self.store_object_name)

        return_dict = {identifier: value for identifier, value in variables.get()}
        self.df = self.df.append(pd.DataFrame(return_dict, index=[self.event_number_counter]))

        self.event_number_counter += 1

    def terminate(self):
        """
        Save the internal pandas data frame to a ROOT file.
        """
        to_root(self.df, self.store_object_name)
