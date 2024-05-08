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
                'data_MC_uncertainty_sys_up',
                # 'data_MC_uncertainty_statsys_dn',
                # 'data_MC_uncertainty_statsys_up',
                # 'rel_data_MC_uncertainty_stat_dn',
                # 'rel_data_MC_uncertainty_stat_up','rel_data_MC_uncertainty_sys_dn',
                # 'rel_data_MC_uncertainty_sys_up','rel_data_MC_uncertainty_statsys_dn',
                # 'rel_data_MC_uncertainty_statsys_up'
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

    def __init__(self, n_variations: int = 100) -> None:
        self.n_variations = n_variations
        self.particles = []
        self.correlations = []

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
        binning_df = binning_df.merge(particle.merged_table[_weight_cols + binning_df.columns.tolist()],
                                      on=binning_df.columns.tolist(), how='left')
        for col in _weight_cols:
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
        self.particles += [ReweighterParticle(prefix,
                                              merged_table=merged_weight_df,
                                              pdg_binning=pdg_binning,
                                              pid_threshold={pid_variable_name: threshold},
                                              variable_aliases=variable_aliases)]

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


if __name__ == "__main__":
    import uproot
    with uproot.open("~/belle2/sutclw_b_sll_bonn/AnalysisSelections/mc15v16B0eeB0/Upsilon4SB0candee.root") as f:
        df = f["variables"].arrays(library="pd")
    pv_path = '~/belle2/pidvar/examples/'
    efftable_e = pd.read_csv(f'{pv_path}/v0b/efficiency/e_efficiency_table.csv')
    efftable_mu = pd.read_csv(f'{pv_path}/v0b/efficiency/mu_efficiency_table.csv')
    faketable_pi_e = pd.read_csv(f'{pv_path}/v0b/fakeRate/pi_e_fakeRate_table.csv')
    faketable_pi_mu = pd.read_csv(f'{pv_path}/v0b/fakeRate/pi_mu_fakeRate_table.csv')
    faketable_K_e = pd.read_csv(f'{pv_path}/v0b/fakeRate/K_e_fakeRate_table.csv')
    faketable_K_mu = pd.read_csv(f'{pv_path}/v0b/fakeRate/K_mu_fakeRate_table.csv')

    best_available_query = "is_best_available == True"
    exclude_e_bins_query = "not (theta_min == 0.56 and theta_max == 2.23) "
    exclude_e_bins_query += "and not (theta_min == 0.22 and theta_max == 2.71)"
    exclude_e_bins_query += "and not (p_min == 0.2 and p_max == 7.0)"
    exclude_mu_bins_query = "not (theta_min == 0.82 and theta_max == 2.22) "
    exclude_mu_bins_query += "and not (theta_min == 0.4 and theta_max == 0.82) "
    exclude_mu_bins_query += "and not (theta_min == 0.4 and theta_max == 2.6)"
    e_query = f"({best_available_query}) and ({exclude_e_bins_query})"
    mu_query = f"({best_available_query}) and ({exclude_mu_bins_query})"
    fake_query = f"({best_available_query}) and not (theta_min == 0.22 and theta_max == 2.71) "
    fake_query += " and not (theta_min == 0.56 and theta_max == 2.23)"
    fake_query += " and not (theta_min == 0.82 and theta_max == 2.22)"
    fake_query += " and not (theta_min == 0.4 and theta_max == 2.6)"
    fake_query += " and not (theta_min == 0.4 and theta_max == 0.82)"
    fake_query += " and not (p_min == 0.2 and p_max == 5.)"

    efftables = {11: efftable_e.query(e_query), 13: efftable_mu.query(mu_query)}
    faketables = {
        211: faketable_pi_e.query(fake_query),
        # (211,13) :faketable_pi_mu.query(fake_query),
        321: faketable_K_e.query(fake_query),
        # (321,13) :faketable_K_mu.query(fake_query)
    }
    df['lepminus_charge'] = -1
    df['lepplus_charge'] = 1
    df['lepplus_mcPDG'] = df['lepplus_MC_PDG'].fillna(0).astype(int)
    df['lepminus_mcPDG'] = df['lepminus_MC_PDG'].fillna(0).astype(int)
    reweighter = Reweighter()
    reweighter.add_pid_particle({11: efftable_e.query(e_query)}, faketables, 'pidChargedBDTScore_e', 0.9, 'lepminus_')
    reweighter.add_pid_particle({11: efftable_e.query(e_query)}, faketables, 'pidChargedBDTScore_e', 0.9, 'lepplus_')
    reweighter.reweight(df)
    print(reweighter.get_particle('lepminus_'))
    print(df.columns)
