import pandas as pd
import numpy as np
from dataclasses import dataclass


"""
A module that adds corrections to analysis dataframe.
It adds weight variations according to the total uncertainty for easier error propagation.
"""

_weight_cols = ['data_MC_ratio',
                'data_MC_uncertainty_stat_dn',
                'data_MC_uncertainty_stat_up',
                'data_MC_uncertainty_sys_dn',
                'data_MC_uncertainty_sys_up'
                ]


@dataclass
class ReweighterParticle:
    """
    Class that stores the information of a particle.
    """
    prefix: str
    merged_table: pd.DataFrame
    pdg_binning: dict
    pid_threshold: dict
    variable_aliases: dict
    weight_name: str
    column_names: list = None

    def generate_variations(self,
                            n_variations: int,
                            syscorr: bool = True,
                            rho_sys: np.ndarray = None,
                            rho_stat: np.ndarray = None) -> None:
        """
        Generates variations of weights according to the uncertainties
        """
        self.merged_table['stat_error'] = self.merged_table[["data_MC_uncertainty_stat_up",
                                                             "data_MC_uncertainty_stat_dn"]].max(axis=1)
        self.merged_table['sys_error'] = self.merged_table[["data_MC_uncertainty_sys_up",
                                                            "data_MC_uncertainty_sys_dn"]].max(axis=1)
        self.merged_table["error"] = np.sqrt(self.merged_table["stat_error"] ** 2 + self.merged_table["sys_error"] ** 2)
        means = self.merged_table["data_MC_ratio"].values
        self.column_names = [f"{self.weight_name}_{i}" for i in range(n_variations)]
        if rho_sys is None:
            if syscorr:
                rho_sys = np.ones((len(means), len(means)))
            else:
                rho_sys = np.identity(len(means))
        if rho_stat is None:
            rho_stat = np.identity(len(means))

        zeros = np.zeros(len(means))
        sys_cov = np.matmul(
            np.matmul(np.diag(self.merged_table['sys_error']), rho_sys), np.diag(self.merged_table['sys_error'])
        )
        stat_cov = np.matmul(
            np.matmul(np.diag(self.merged_table['stat_error']), rho_stat), np.diag(self.merged_table['stat_error'])
        )
        sys = np.random.multivariate_normal(zeros, sys_cov, n_variations)
        stat = np.random.multivariate_normal(zeros, stat_cov, n_variations)
        weights = sys + stat + means
        self.merged_table[self.weight_name] = self.merged_table["data_MC_ratio"]
        self.merged_table[self.column_names] = weights.T
        self.column_names.insert(0, self.weight_name)

    def __str__(self) -> str:
        """
        Converts the object to a string.
        """
        separator = '------------------'
        title = 'ReweighterParticle'
        prefix_str = f'Prefix: {self.prefix}'
        merged_table_str = f'Merged table:\n{self.merged_table.describe()}'
        pdg_binning_str = 'PDG binning:\n'
        for pdg in self.pdg_binning:
            pdg_binning_str += f'{pdg}: {self.pdg_binning[pdg]}\n'
        return f'{separator}\n{title}\n{prefix_str}\n{merged_table_str}\n{pdg_binning_str}\n{separator}'


class Reweighter:
    """
    Class that reweights the dataframe.
    """

    def __init__(self, n_variations: int = 100, weight_name: str = "Weight") -> None:
        self.n_variations = n_variations
        self.particles = []
        self.correlations = []
        self.weight_name = weight_name
        self.weights_generated = False

    def get_bin_columns(self, weight_df) -> list:
        """
        Returns the kinematic bin columns of the dataframe.
        """
        return [col for col in weight_df.columns if col.endswith('_min') or col.endswith('_max')]

    def get_binning(self, weight_df) -> dict:
        """
        Returns the kinematic binning of the dataframe.
        """
        columns = self.get_bin_columns(weight_df)
        var_names = {'_'.join(col.split('_')[:-1]) for col in columns}
        bin_dict = {}
        for var_name in var_names:
            bin_dict[var_name] = []
            for col in columns:
                if col.startswith(var_name):
                    bin_dict[var_name] += list(weight_df[col].values)
            bin_dict[var_name] = np.array(sorted(set(bin_dict[var_name])))
        return bin_dict

    def get_ntuple_variables(self,
                             ntuple_df: pd.DataFrame,
                             bin_variables: list,
                             pid_variable_name: str,
                             mc_pdg_name: str,
                             prefix: str) -> None:
        """
        Checks if the variables are in the ntuple and returns them.
        Parameters:
            ntuple_df (pd.DataFrame): Dataframe containing the analysis ntuple.
            bin_variables (list): List of kinematic bin variables.
            pid_variable_name (str): Name of the PID variable.
            prefix (str): Prefix for the new columns.
        """
        ntuple_variables = [f'{prefix}_{var}' for var in bin_variables]
        if pid_variable_name:
            ntuple_variables += [f'{prefix}_{pid_variable_name}']
        if mc_pdg_name:
            ntuple_variables += [f'{prefix}_{mc_pdg_name}']
        for var in ntuple_variables:
            if var not in ntuple_df.columns:
                raise ValueError(f'Variable {var} not in ntuple! Required variables are {ntuple_variables}')
        return ntuple_variables

    def merge_weight_tables(self,
                            eff_weights_dict: dict,
                            fake_weights_dict: dict,
                            pid_variable_name: str,
                            threshold: float) -> pd.DataFrame:
        """
        Merges the efficiency and fake rate weight tables.
        Parameters:
            eff_weights_df (pd.DataFrame): Dataframe containing the efficiency weights.
            fake_weights_dict (pd.DataFrame): Dataframe containing the fake rate weights.
            mc_pdg_name (str): Name of the MC particle PDG ID.
        """
        weight_dfs = []
        for pdg in fake_weights_dict:
            fake_weights_dict[pdg]['mcPDG'] = pdg
            weight_dfs.append(fake_weights_dict[pdg].query(f'variable == "{pid_variable_name}" and threshold == {threshold}'))
        for pdg in eff_weights_dict:
            eff_weights_dict[pdg]['mcPDG'] = pdg
            weight_dfs.append(eff_weights_dict[pdg].query(f'variable == "{pid_variable_name}" and threshold == {threshold}'))
        return pd.concat(weight_dfs, ignore_index=True)

    def add_weight_columns(self,
                           ntuple_df: pd.DataFrame,
                           particle: ReweighterParticle) -> None:
        """
        Adds a weight and uncertainty columns to the dataframe.
        Parameters:
            ntuple_df (pd.DataFrame): Dataframe containing the analysis ntuple.
            particle (ReweighterParticle): Particle object.
        """
        # Apply a weight value from the weight table to the ntuple, based on the binning
        binning_df = pd.DataFrame(index=ntuple_df.index)
        # Take absolute value of mcPDG for binning because we have charge already
        binning_df['mcPDG'] = ntuple_df[f'{particle.prefix}mcPDG'].abs()
        for pdg in particle.pdg_binning:
            for var in particle.pdg_binning[pdg]:
                print(pdg, var)
                labels = [(particle.pdg_binning[pdg][var][i-1], particle.pdg_binning[pdg][var][i])
                          for i in range(1, len(particle.pdg_binning[pdg][var]))]
                binning_df.loc[binning_df['mcPDG'] == pdg, var] = pd.cut(ntuple_df.query(f'abs({particle.prefix}mcPDG) == {pdg}')[
                                                                        f'{particle.prefix}{var}'],
                                                                        particle.pdg_binning[pdg][var],
                                                                        labels=labels)
                print(binning_df.query(f'mcPDG == {pdg}')[var])
                print(ntuple_df.query(f'{particle.prefix}mcPDG == {pdg}')[f'{particle.prefix}{var}'])
                binning_df.loc[binning_df['mcPDG'] == pdg,
                               f'{var}_min'] = binning_df.loc[binning_df['mcPDG'] == pdg,
                                                              var].str[0]  # Uh oh
                binning_df.loc[binning_df['mcPDG'] == pdg,
                               f'{var}_max'] = binning_df.loc[binning_df['mcPDG'] == pdg,
                                                              var].str[1]  # Oh uff
                binning_df.drop(var, axis=1, inplace=True)
        # merge the weight table with the ntuple on binning columns
        weight_cols = _weight_cols
        if particle.column_names:
            weight_cols = particle.column_names
        binning_df = binning_df.merge(particle.merged_table[weight_cols + binning_df.columns.tolist()],
                                      on=binning_df.columns.tolist(), how='left')
        for col in weight_cols:
            ntuple_df[f'{particle.prefix}{col}'] = binning_df[col]

    def add_pid_particle(self,
                         eff_weights_dict: dict,
                         fake_weights_dict: dict,
                         pid_variable_name: str,
                         threshold: float,
                         prefix: str,
                         variable_aliases: dict = None) -> None:
        """
        Adds weight variations according to the total uncertainty for easier error propagation.
        Parameters:
            ntuple_df (pd.DataFrame): Dataframe containing the analysis ntuple.
            eff_weights_dict (pd.DataFrame): Dataframe containing the efficiency weights.
            fake_weights_dict (pd.DataFrame): Dataframe containing the fake rate weights.
            pid_variable_name (str): Name of the PID variable.
            threshold (float): Threshold for the PID variable.
            prefix (str): Prefix for the new columns.
        """
        # Empty prefix means no prefix
        if prefix is None:
            prefix = ''
        # Add underscore if not present
        if prefix and not prefix.endswith('_'):
            prefix += '_'
        merged_weight_df = self.merge_weight_tables(eff_weights_dict, fake_weights_dict, pid_variable_name, threshold)
        pdg_binning = {pdg: self.get_binning(merged_weight_df.query(f'mcPDG == {pdg}'))
                       for pdg in merged_weight_df['mcPDG'].unique()}
        # Check if these are legacy tables:
        if 'charge' in merged_weight_df.columns:
            charge_dict = {'+': [0, 2], '-': [-2, 0]}
            merged_weight_df[['charge_min', 'charge_max']] = [charge_dict[val] for val in merged_weight_df['charge'].values]
            merged_weight_df = merged_weight_df.drop(columns=['charge'])
            for pdg in pdg_binning:
                pdg_binning[pdg]['charge'] = np.array([-2, 0, 2])
                if 'iso_score' in pdg_binning[pdg] and len(pdg_binning[pdg]['iso_score']) == 2:
                    del pdg_binning[pdg]['iso_score']
        if not variable_aliases:
            variable_aliases = {pid_variable_name: pid_variable_name}
        particle = ReweighterParticle(prefix,
                                      merged_table=merged_weight_df,
                                      pdg_binning=pdg_binning,
                                      pid_threshold={pid_variable_name: threshold},
                                      variable_aliases=variable_aliases,
                                      weight_name=self.weight_name)
        particle.generate_variations(n_variations=self.n_variations)
        self.particles += [particle]

    def get_particle(self, prefix: str) -> ReweighterParticle:
        """
        Get a particle by its prefix.
        """
        cands = [particle for particle in self.particles if particle.prefix.strip('_') == prefix.strip('_')]
        if len(cands) == 0:
            raise ValueError(f"No particle with prefix {prefix.strip('_')} found.")
        return cands[0]

    def add_correlation(self, prefix_left, prefix_right, rho: float):
        """
        Add a correlation between two PID particles.
        """
        particle_left = self.get_particle(prefix_left)
        particle_right = self.get_particle(prefix_right)
        self.correlations += [(particle_left, particle_right, rho)]

    def reweight(self, df: pd.DataFrame):
        """
        Reweights the dataframe according to the weight tables.
        Parameters:
            df (pd.DataFrame): Dataframe containing the analysis ntuple.
        """
        for particle in self.particles:
            self.add_weight_columns(df, particle)
        return df
