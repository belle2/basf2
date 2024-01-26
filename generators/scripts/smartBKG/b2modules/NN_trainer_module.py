##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import os
import shutil
import pandas as pd
import awkward as ak

import basf2 as b2
from ROOT import Belle2
import modularAnalysis as ma
from skim.WGs.fei import feiHadronicB0
from b2pandas_utils import VariablesToHDF5

from smartBKG.utils.preprocess import load_particle_list, preprocessed


class SaveFlag(b2.Module):
    """
    Save event numbers to a Parquet file.

    Arguments:
        out_file (str): Output file path for saving the event numbers.

    Returns:
        None

    Note:
        This module should be added after the skimming process.
    """

    def __init__(self, out_file=None):
        """
        Initialize the SaveFlag module.

        :param out_file: Output file path for saving the event numbers.
        """
        super().__init__()
        #: Output file path for saving the event numbers
        self.out_file = out_file

    def initialize(self):
        """
        Initialize the data store and the list to save event numbers before processing events.
        """
        #: Initialise event metadata from data store
        self.eventInfo = Belle2.PyStoreObj('EventMetaData')
        #: List to save event numbers of pass events
        self.pass_list = []

    def event(self):
        """
        Process each event and append event numbers to the pass list.
        """
        self.pass_list.append(self.eventInfo.getEvent())

    def terminate(self):
        """
        Finalize the module and save the pass list to a Parquet file.
        """
        ak.to_parquet(self.pass_list, self.out_file)


class TrainDataSaver(b2.Module):
    """
    Save MCParticles to Pandas Dataframe.

    Arguments:
        output_file (str): Filename to save training data to.
        flag_file (str): Filename of the flag file indicating passing events.

    Returns:
        None
    """

    def __init__(
        self,
        output_file,
        flag_file,
    ):
        """
        Initialize the TrainDataSaver module.

        :param output_file: Filename to save training data to.
        :param flag_file: Filename of the flag file indicating passing events.
        """
        super().__init__()
        #: Filename to save training data to
        self.output_file = output_file
        #: Filename of the flag file indicating passing events
        self.flag_list = ak.from_parquet(flag_file)

        # delete output file if it already exists, since we will apend later
        if os.path.exists(output_file):
            os.remove(output_file)

    def initialize(self):
        """
        Initialize the data store and the dictionary to save particle features before processing events.
        """
        #: Initialise event metadata from data store
        self.eventinfo = Belle2.PyStoreObj('EventMetaData')
        #: Initialise event extra info from data store
        self.eventExtraInfo = Belle2.PyStoreObj('EventExtraInfo')
        #: Pandas dataframe to save particle features
        self.df_dict = pd.DataFrame()
        # #: record the production time(s) of root particle(s) for the correction of jitter
        # self.root_prodTime = defaultdict(list)

    def event(self):
        """
        Process each event and append event information to the dictionary.
        """
        evtNum = self.eventinfo.getEvent()
        self.df_dict = pd.concat([
            self.df_dict,
            load_particle_list(mcplist=Belle2.PyStoreArray("MCParticles"), evtNum=evtNum, label=(evtNum in self.flag_list))
            ])

    def terminate(self):
        """
        Append events to the DataFrame on disk and free memory.
        """
        self.df_dict.to_hdf(self.output_file, key='mc_information', mode='a', format='table', append=True)
        self.df_dict = pd.DataFrame()


class data_production():
    """
    Process data for training and save to Parquet file.

    Arguments:
        in_dir (str): Input directory.
        out_dir (str): Output directory.
        job_id (int): Job ID for batch processing.
        save_vars (dict): Variables to save for different event levels.
            By default having Y4S and B keys for the corresponding particle list.
        dataName (str): Data file name prefix.
        flagName (str): Flag file name prefix.

    Returns:
        None
    """

    def __init__(self, in_dir, out_dir, job_id, save_vars=None):
        """
        Initialize the data_production object.

        :param in_dir: Input directory.
        :param out_dir: Output directory.
        :param job_id: Job ID for batch processing.
        :param save_vars: Variables to save for different event levels.
            By default having Y4S and B keys for the corresponding particle list.
        """
        dataName = '_submdst'
        flagName = '_flag'
        #: Input root file generated before skimming
        self.data = f'{in_dir}{dataName}{job_id}.root'
        #: Filename of the flag file indicating passing events
        self.flag = f'{in_dir}{flagName}{job_id}.parquet'
        #: Temperary directory to keep intermediate files
        self.out_temp = f'{out_dir}_temp{job_id}/'
        os.makedirs(out_dir, exist_ok=True)
        os.makedirs(self.out_temp, exist_ok=True)
        #: Intermediate files
        self.temp_file = {
            'MC': f'{self.out_temp}mc.h5',
            'Y4S': f'{self.out_temp}y4s.h5',
            'B': f'{self.out_temp}b.h5'
            }
        #: Final output Parquet file
        self.out_file = f'{out_dir}preprocessed{job_id}.parquet'
        #: Variables to save for different event levels
        self.save_vars = save_vars

    def process(self):
        """
        Process the b2 steering file and the data generation.
        """
        self.process_b2script()
        self.preprocess_files()

    def process_b2script(self, num_events=2500):
        """
        Skimming process with TrainDataSaver module.

        :param num_events: Maximum number of events to process.
        """
        path = ma.create_path()

        ma.inputMdst(environmentType='default', filename=self.data, path=path)
        ma.buildEventShape(path=path)
        ma.buildEventKinematics(path=path)

        TrainDataSaver_module = TrainDataSaver(
            output_file=self.temp_file['MC'],
            flag_file=self.flag
        )
        path.add_module(TrainDataSaver_module)

        if self.save_vars is not None:
            ma.fillParticleListFromMC('Upsilon(4S):mc', '', path=path)
            v2hdf5_y4s = VariablesToHDF5(
                'Upsilon(4S):mc',
                self.save_vars['Y4S'],
                filename=self.temp_file['Y4S'],
            )
            path.add_module(v2hdf5_y4s)

            fei_skim = feiHadronicB0(udstOutput=False, analysisGlobaltag=ma.getAnalysisGlobaltag())
            fei_skim(path=path)
            fei_skim.postskim_path.add_module(
                    "BestCandidateSelection",
                    particleList="B0:generic",
                    variable="extraInfo(SignalProbability)",
                    outputVariable="rank_signalprob",
                    numBest=1,
                )
            # Key of saved table is the name of particle list
            v2hdf5_b = VariablesToHDF5(
                'B0:generic',
                self.save_vars['B'],
                filename=self.temp_file['B'],
            )
            fei_skim.postskim_path.add_module(v2hdf5_b)

        b2.process(path, max_event=num_events)
        print(b2.statistics)

    def preprocess_files(self):
        """
        Merge decorrelation to DataFrame.
        """
        df = pd.read_hdf(self.temp_file['MC'], key='mc_information')
        if self.save_vars is not None:
            df_y4s = pd.read_hdf(self.temp_file['Y4S'], key='Upsilon(4S):mc')
            df_b = pd.read_hdf(self.temp_file['B'], key='B0:generic')
            df_merged = df_y4s.merge(df_b.drop(axis=1, labels=['icand', 'ncand']), how="left")
            decorr_df = df_merged.rename({'evt': 'evtNum'}, axis=1)
        else:
            decorr_df = None
        df = preprocessed(df, decorr_df)
        ak.to_parquet(df, self.out_file)

    def clean_up(self):
        """
        Clean up temporary files.
        """
        os.remove(self.data)
        os.remove(self.flag)
        shutil.rmtree(self.out_temp)
