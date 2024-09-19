#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import pandas as pd
import numpy as np
from dataclasses import dataclass
import matplotlib.pyplot as plt
import pdg

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

_correction_types = ['PID', 'FEI']
_fei_mode_col = 'dec_mode'


@dataclass
class ReweighterParticle:
    """
    Class that stores the information of a particle.
    """
    #: Prefix of the particle in the ntuple
    prefix: str

    #: Type of the particle (PID or FEI)
    type: str

    #: Merged table of the weights
    merged_table: pd.DataFrame

    #: Kinematic binning of the weight table per particle
    pdg_binning: dict

    #: Variable aliases of the weight table
    variable_aliases: dict

    #: Weight column name that will be added to the ntuple
    weight_name: str

    #: Internal list of the names of the weight columns
    column_names: list = None

    #: Random seed for systematics
    sys_seed: int = None

    #: Covariance matrix corresponds to the total uncertainty
    cov: np.ndarray = None

    #: When true assume systematics are 100% correlated
    syscorr: bool = True

    #: Coverage of the user ntuple
    coverage: float = None

    #: Values for the plots
    plot_values: dict = None

    def get_varname(self, varname: str) -> str:
        """
        Returns the variable name with the prefix and use alias if defined.
        """
        name = varname
        if self.variable_aliases and varname in self.variable_aliases:
            name = self.variable_aliases[varname]
        if name.startswith(self.prefix):
            return name
        return f'{self.prefix}{name}'

    def get_binning_variables(self) -> list:
        """
        Returns the list of variables that are used for the binning
        """
        variables = set(sum([list(d.keys()) for d in self.pdg_binning.values()], []))
        return [f'{self.get_varname(var)}' for var in variables]

    def get_pdg_variables(self) -> list:
        """
        Returns the list of variables that are used for the PDG codes
        """
        pdg_vars = ['PDG']
        #: Add the mcPDG code requirement for PID particle
        if self.type == "PID":
            pdg_vars += ['mcPDG']
        return [f'{self.get_varname(var)}' for var in pdg_vars]

    def generate_variations(self,
                            n_variations: int,
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
        #: Names of the varied weight columns
        self.column_names = [f"{self.weight_name}_{i}" for i in range(n_variations)]
        cov = self.get_covariance(n_variations, rho_sys, rho_stat)
        weights = cov + means
        self.merged_table[self.weight_name] = self.merged_table["data_MC_ratio"]
        self.merged_table[self.column_names] = weights.T
        self.column_names.insert(0, self.weight_name)

    def get_covariance(self,
                       n_variations: int,
                       rho_sys: np.ndarray = None,
                       rho_stat: np.ndarray = None) -> np.ndarray:
        """
        Returns the covariance matrix of the weights
        """
        len_means = len(self.merged_table["data_MC_ratio"])
        zeros = np.zeros(len_means)
        if self.cov is None:
            if rho_sys is None:
                if self.syscorr:
                    rho_sys = np.ones((len_means, len_means))
                else:
                    rho_sys = np.identity(len_means)
            if rho_stat is None:
                rho_stat = np.identity(len_means)
            sys_cov = np.matmul(
                np.matmul(np.diag(self.merged_table['sys_error']), rho_sys), np.diag(self.merged_table['sys_error'])
            )
            stat_cov = np.matmul(
                np.matmul(np.diag(self.merged_table['stat_error']), rho_stat), np.diag(self.merged_table['stat_error'])
            )
            np.random.seed(self.sys_seed)
            sys = np.random.multivariate_normal(zeros, sys_cov, n_variations)
            np.random.seed(None)
            stat = np.random.multivariate_normal(zeros, stat_cov, n_variations)
            return sys + stat
        errors = np.random.multivariate_normal(zeros, self.cov, n_variations)
        return errors

    def __str__(self) -> str:
        """
        Converts the object to a string.
        """
        separator = '------------------'
        title = 'ReweighterParticle'
        prefix_str = f'Type: {self.type} Prefix: {self.prefix}'
        columns = _weight_cols
        merged_table_str = f'Merged table:\n{self.merged_table[columns].describe()}'
        pdg_binning_str = 'PDG binning:\n'
        for pdgs in self.pdg_binning:
            pdg_binning_str += f'{pdgs}: {self.pdg_binning[pdgs]}\n'
        return '\n'.join([separator, title, prefix_str, merged_table_str, pdg_binning_str]) + separator

    def plot_coverage(self, fig=None, axs=None):
        """
        Plots the coverage of the ntuple.
        """
        if self.plot_values is None:
            return
        vars = set(sum([list(d.keys()) for d in self.plot_values.values()], []))
        if fig is None:
            fig, axs = plt.subplots(len(self.plot_values), len(vars), figsize=(5*len(vars), 3*len(self.plot_values)), dpi=120)
        axs = np.array(axs)
        if len(axs.shape) < 1:
            axs = axs.reshape(len(self.plot_values), len(vars))
        bin_plt = {'linewidth': 3, 'linestyle': '--', 'color': '0.5'}
        fig.suptitle(f'{self.type} particle {self.prefix.strip("_")}')
        for (reco_pdg, mc_pdg), ax_row in zip(self.plot_values, axs):
            for var, ax in zip(self.plot_values[(reco_pdg, mc_pdg)], ax_row):
                ymin = 0
                ymax = self.plot_values[(reco_pdg, mc_pdg)][var][1].max()*1.1
                # Plot binning
                if self.type == 'PID':
                    ax.vlines(self.pdg_binning[(reco_pdg, mc_pdg)][var], ymin, ymax,
                              label='Binning',
                              alpha=0.8,
                              **bin_plt)
                elif self.type == 'FEI':
                    values = np.array([int(val[4:]) for val in self.pdg_binning[(reco_pdg, mc_pdg)][var]])
                    ax.bar(values+0.5,
                           np.ones(len(values))*ymax,
                           width=1,
                           alpha=0.5,
                           label='Binning',
                           **bin_plt)
                    rest = np.setdiff1d(self.plot_values[(reco_pdg, mc_pdg)][var][0], values)
                    ax.bar(rest+0.5,
                           np.ones(len(rest))*ymax,
                           width=1,
                           alpha=0.2,
                           label='Rest category',
                           **bin_plt)
                # Plot values
                widths = (self.plot_values[(reco_pdg, mc_pdg)][var][0][1:] - self.plot_values[(reco_pdg, mc_pdg)][var][0][:-1])
                centers = self.plot_values[(reco_pdg, mc_pdg)][var][0][:-1] + widths/2
                ax.bar(centers,
                       self.plot_values[(reco_pdg, mc_pdg)][var][1],
                       width=widths,
                       label='Values',
                       alpha=0.8)
                ax.set_title(f'True {pdg.to_name(mc_pdg)} to reco {pdg.to_name(reco_pdg)} coverage')
                ax.set_xlabel(var)
        axs[-1][-1].legend()
        fig.tight_layout()
        return fig, axs


class Reweighter:
    """
    Class that reweights the dataframe.

    Args:
        n_variations (int): Number of weight variations to generate.
        weight_name (str): Name of the weight column.
        evaluate_plots (bool): Flag to indicate if the plots should be evaluated.
        nbins (int): Number of bins for the plots.
    """

    def __init__(self,
                 n_variations: int = 100,
                 weight_name: str = "Weight",
                 evaluate_plots: bool = True,
                 nbins: int = 50,
                 fillna: float = 1.0) -> None:
        """
        Initializes the Reweighter class.
        """
        #: Number of weight variations to generate
        self.n_variations = n_variations
        #: List of particles
        self.particles = []
        #: Correlations between the particles
        self.correlations = []
        #: Name of the weight column
        self.weight_name = weight_name
        #: Flag to indicate if the weights have been generated
        self.weights_generated = False
        #: Flag to indicate if the plots should be evaluated
        self.evaluate_plots = evaluate_plots
        #: Number of bins for the plots
        self.nbins = nbins
        #: Value to fill NaN values
        self.fillna = fillna

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

    def get_fei_binning(self, weight_df) -> dict:
        """
        Returns the irregular binning of the dataframe.
        """
        return {_fei_mode_col: weight_df.loc[weight_df[_fei_mode_col].str.startswith('mode'),
                                             _fei_mode_col].value_counts().index.to_list()}

    def get_ntuple_variables(self,
                             ntuple_df: pd.DataFrame,
                             particle: ReweighterParticle) -> None:
        """
        Checks if the variables are in the ntuple and returns them.

        Args:
            ntuple_df (pandas.DataFrame): Dataframe containing the analysis ntuple.
            particle (ReweighterParticle): Particle object containing the necessary variables.
        """
        ntuple_variables = particle.get_binning_variables()
        ntuple_variables += particle.get_pdg_variables()
        for var in ntuple_variables:
            if var not in ntuple_df.columns:
                raise ValueError(f'Variable {var} is not in the ntuple! Required variables are {ntuple_variables}')
        return ntuple_variables

    def merge_pid_weight_tables(self,
                                weights_dict: dict,
                                pdg_pid_variable_dict: dict) -> pd.DataFrame:
        """
        Merges the efficiency and fake rate weight tables.

        Args:
            weights_dict (dict): Dictionary containing the weight tables.
            pdg_pid_variable_dict (dict): Dictionary containing the PDG codes and variable names.
        """
        weight_dfs = []
        for reco_pdg, mc_pdg in weights_dict:
            if reco_pdg not in pdg_pid_variable_dict:
                raise ValueError(f'Reconstructed PDG code {reco_pdg} not found in thresholds!')
            weight_df = weights_dict[(reco_pdg, mc_pdg)]
            weight_df['mcPDG'] = mc_pdg
            weight_df['PDG'] = reco_pdg
            # Check if these are legacy tables:
            if 'charge' in weight_df.columns:
                charge_dict = {'+': [0, 2], '-': [-2, 0]}
                weight_df[['charge_min', 'charge_max']] = [charge_dict[val] for val in weight_df['charge'].values]
                weight_df = weight_df.drop(columns=['charge'])
                # If iso_score is a single value, drop the min and max columns
                if 'iso_score_min' in weight_df.columns and len(weight_df['iso_score_min'].unique()) == 1:
                    weight_df = weight_df.drop(columns=['iso_score_min', 'iso_score_max'])
            pid_variable_name = pdg_pid_variable_dict[reco_pdg][0]
            threshold = pdg_pid_variable_dict[reco_pdg][1]
            selected_weights = weight_df.query(f'variable == "{pid_variable_name}" and threshold == {threshold}')
            if len(selected_weights) == 0:
                available_variables = weight_df['variable'].unique()
                available_thresholds = weight_df['threshold'].unique()
                raise ValueError(f'No weights found for PDG code {reco_pdg}, mcPDG {mc_pdg},'
                                 f' variable {pid_variable_name} and threshold {threshold}!\n'
                                 f' Available variables: {available_variables}\n'
                                 f' Available thresholds: {available_thresholds}')
            weight_dfs.append(selected_weights)
        return pd.concat(weight_dfs, ignore_index=True)

    def add_pid_weight_columns(self,
                               ntuple_df: pd.DataFrame,
                               particle: ReweighterParticle) -> None:
        """
        Adds a weight and uncertainty columns to the dataframe.

        Args:
            ntuple_df (pandas.DataFrame): Dataframe containing the analysis ntuple.
            particle (ReweighterParticle): Particle object.
        """
        # Apply a weight value from the weight table to the ntuple, based on the binning
        binning_df = pd.DataFrame(index=ntuple_df.index)
        # Take absolute value of mcPDG for binning because we have charge already
        binning_df['mcPDG'] = ntuple_df[f'{particle.get_varname("mcPDG")}'].abs()
        binning_df['PDG'] = ntuple_df[f'{particle.get_varname("PDG")}'].abs()
        plot_values = {}
        for reco_pdg, mc_pdg in particle.pdg_binning:
            ntuple_cut = f'abs({particle.get_varname("mcPDG")}) == {mc_pdg} and abs({particle.get_varname("PDG")}) == {reco_pdg}'
            if ntuple_df.query(ntuple_cut).empty:
                continue
            plot_values[(reco_pdg, mc_pdg)] = {}
            for var in particle.pdg_binning[(reco_pdg, mc_pdg)]:
                labels = [(particle.pdg_binning[(reco_pdg, mc_pdg)][var][i-1], particle.pdg_binning[(reco_pdg, mc_pdg)][var][i])
                          for i in range(1, len(particle.pdg_binning[(reco_pdg, mc_pdg)][var]))]
                binning_df.loc[(binning_df['mcPDG'] == mc_pdg) & (binning_df['PDG'] == reco_pdg), var] = pd.cut(ntuple_df.query(
                    ntuple_cut)[f'{particle.get_varname(var)}'],
                    particle.pdg_binning[(reco_pdg, mc_pdg)][var], labels=labels)
                binning_df.loc[(binning_df['mcPDG'] == mc_pdg) & (binning_df['PDG'] == reco_pdg),
                               f'{var}_min'] = binning_df.loc[(binning_df['mcPDG'] == mc_pdg) & (binning_df['PDG'] == reco_pdg),
                                                              var].str[0]
                binning_df.loc[(binning_df['mcPDG'] == mc_pdg) & (binning_df['PDG'] == reco_pdg),
                               f'{var}_max'] = binning_df.loc[(binning_df['mcPDG'] == mc_pdg) & (binning_df['PDG'] == reco_pdg),
                                                              var].str[1]
                binning_df.drop(var, axis=1, inplace=True)
                if self.evaluate_plots:
                    values = ntuple_df.query(ntuple_cut)[f'{particle.get_varname(var)}']
                    if len(values.unique()) < 2:
                        print(f'Skip {var} for plotting!')
                        continue
                    x_range = np.linspace(values.min(), values.max(), self.nbins)
                    plot_values[(reco_pdg, mc_pdg)][var] = x_range, np.histogram(values, bins=x_range, density=True)[0]
        # merge the weight table with the ntuple on binning columns
        weight_cols = _weight_cols
        if particle.column_names:
            weight_cols = particle.column_names
        binning_df = binning_df.merge(particle.merged_table[weight_cols + binning_df.columns.tolist()],
                                      on=binning_df.columns.tolist(), how='left')
        binning_df.index = ntuple_df.index
        particle.coverage = 1 - binning_df[weight_cols[0]].isna().sum() / len(binning_df)
        particle.plot_values = plot_values
        for col in weight_cols:
            ntuple_df[f'{particle.get_varname(col)}'] = binning_df[col]
            ntuple_df[f'{particle.get_varname(col)}'] = ntuple_df[f'{particle.get_varname(col)}'].fillna(self.fillna)

    def add_pid_particle(self,
                         prefix: str,
                         weights_dict: dict,
                         pdg_pid_variable_dict: dict,
                         variable_aliases: dict = None,
                         sys_seed: int = None,
                         syscorr: bool = True) -> None:
        """
        Adds weight variations according to the total uncertainty for easier error propagation.

        Args:
            prefix (str): Prefix for the new columns.
            weights_dict (pandas.DataFrame): Dataframe containing the efficiency weights.
            pdg_pid_variable_dict (dict): Dictionary containing the PID variables and thresholds.
            variable_aliases (dict): Dictionary containing variable aliases.
            sys_seed (int): Seed for the systematic variations.
            syscorr (bool): When true assume systematics are 100% correlated defaults to
        true. Note this is overridden by provision of a None value rho_sys
        """
        # Empty prefix means no prefix
        if prefix is None:
            prefix = ''
        # Add underscore if not present
        if prefix and not prefix.endswith('_'):
            prefix += '_'
        if self.get_particle(prefix):
            raise ValueError(f"Particle with prefix '{prefix}' already exists!")
        if variable_aliases is None:
            variable_aliases = {}
        merged_weight_df = self.merge_pid_weight_tables(weights_dict, pdg_pid_variable_dict)
        pdg_binning = {(reco_pdg, mc_pdg): self.get_binning(merged_weight_df.query(f'PDG == {reco_pdg} and mcPDG == {mc_pdg}'))
                       for reco_pdg, mc_pdg in merged_weight_df[['PDG', 'mcPDG']].value_counts().index.to_list()}
        particle = ReweighterParticle(prefix,
                                      type='PID',
                                      merged_table=merged_weight_df,
                                      pdg_binning=pdg_binning,
                                      variable_aliases=variable_aliases,
                                      weight_name=self.weight_name,
                                      sys_seed=sys_seed,
                                      syscorr=syscorr)
        self.particles += [particle]

    def get_particle(self, prefix: str) -> ReweighterParticle:
        """
        Get a particle by its prefix.
        """
        cands = [particle for particle in self.particles if particle.prefix.strip('_') == prefix.strip('_')]
        if len(cands) == 0:
            return None
        return cands[0]

    def convert_fei_table(self, table: pd.DataFrame, threshold: float):
        """
        Checks if the tables are provided in a legacy format and converts them to the standard format.
        """
        result = None
        str_to_pdg = {'B+': 521, 'B-': 521, 'B0': 511}
        if 'cal' in table.columns:
            result = pd.DataFrame(index=table.index)
            result['data_MC_ratio'] = table['cal']
            result['PDG'] = table['Btag'].apply(lambda x: str_to_pdg.get(x))
            # Assume these are only efficiency tables
            result['mcPDG'] = result['PDG']
            result['threshold'] = table['sig_prob_threshold']
            result[_fei_mode_col] = table[_fei_mode_col]
            result['data_MC_uncertainty_stat_dn'] = table['cal_stat_error']
            result['data_MC_uncertainty_stat_up'] = table['cal_stat_error']
            result['data_MC_uncertainty_sys_dn'] = table['cal_sys_error']
            result['data_MC_uncertainty_sys_up'] = table['cal_sys_error']
        elif 'cal factor' in table.columns:
            result = pd.DataFrame(index=table.index)
            result['data_MC_ratio'] = table['cal factor']
            result['PDG'] = table['Btype'].apply(lambda x: str_to_pdg.get(x))
            result['mcPDG'] = result['PDG']
            result['threshold'] = table['sig prob cut']
            # Assign the total error to the stat uncertainty and set syst. one to 0
            result['data_MC_uncertainty_stat_dn'] = table['error']
            result['data_MC_uncertainty_stat_up'] = table['error']
            result['data_MC_uncertainty_sys_dn'] = 0
            result['data_MC_uncertainty_sys_up'] = 0
            result[_fei_mode_col] = table['mode']
        else:
            result = table
        result = result.query(f'threshold == {threshold}')
        if len(result) == 0:
            raise ValueError(f'No weights found for threshold {threshold}!')
        return result

    def add_fei_particle(self, prefix: str,
                         table: pd.DataFrame,
                         threshold: float,
                         cov: np.ndarray = None,
                         variable_aliases: dict = None,
                         ) -> None:
        """
        Adds weight variations according to the total uncertainty for easier error propagation.

        Args:
            prefix (str): Prefix for the new columns.
            table (pandas.DataFrame): Dataframe containing the efficiency weights.
            threshold (float): Threshold for the efficiency weights.
            cov (numpy.ndarray): Covariance matrix for the efficiency weights.
            variable_aliases (dict): Dictionary containing variable aliases.
        """
        # Empty prefix means no prefix
        if prefix is None:
            prefix = ''
        if prefix and not prefix.endswith('_'):
            prefix += '_'
        if self.get_particle(prefix):
            raise ValueError(f"Particle with prefix '{prefix}' already exists!")
        if variable_aliases is None:
            variable_aliases = {}
        if table is None or len(table) == 0:
            raise ValueError('No weights provided!')
        converted_table = self.convert_fei_table(table, threshold)
        pdg_binning = {(reco_pdg, mc_pdg): self.get_fei_binning(converted_table.query(f'PDG == {reco_pdg} and mcPDG == {mc_pdg}'))
                       for reco_pdg, mc_pdg in converted_table[['PDG', 'mcPDG']].value_counts().index.to_list()}
        particle = ReweighterParticle(prefix,
                                      type='FEI',
                                      merged_table=converted_table,
                                      pdg_binning=pdg_binning,
                                      variable_aliases=variable_aliases,
                                      weight_name=self.weight_name,
                                      cov=cov)
        self.particles += [particle]

    def add_fei_weight_columns(self, ntuple_df: pd.DataFrame, particle: ReweighterParticle):
        """
        Adds weight columns according to the FEI calibration tables
        """
        rest_str = 'rest'
        particle.merged_table[_fei_mode_col]
        # Apply a weight value from the weight table to the ntuple, based on the binning
        binning_df = pd.DataFrame(index=ntuple_df.index)
        # Take absolute value of mcPDG for binning because we have charge already
        binning_df['PDG'] = ntuple_df[f'{particle.get_varname("PDG")}'].abs()
        # Copy the mode ID from the ntuple
        binning_df['num_mode'] = ntuple_df[particle.get_varname(_fei_mode_col)].astype(int)
        # Default value in case if reco PDG is not a B-meson PDG
        binning_df[_fei_mode_col] = np.nan
        plot_values = {}
        for reco_pdg, mc_pdg in particle.pdg_binning:
            plot_values[(reco_pdg, mc_pdg)] = {}
            binning_df.loc[binning_df['PDG'] == reco_pdg, _fei_mode_col] = particle.merged_table.query(
                f'PDG == {reco_pdg} and {_fei_mode_col}.str.lower() == "{rest_str}"')[_fei_mode_col].values[0]
            for mode in particle.pdg_binning[(reco_pdg, mc_pdg)][_fei_mode_col]:
                binning_df.loc[(binning_df['PDG'] == reco_pdg) & (binning_df['num_mode'] == int(mode[4:])), _fei_mode_col] = mode
            if self.evaluate_plots:
                values = ntuple_df[f'{particle.get_varname(_fei_mode_col)}']
                x_range = np.linspace(values.min(), values.max(), int(values.max())+1)
                plot_values[(reco_pdg, mc_pdg)][_fei_mode_col] = x_range, np.histogram(values, bins=x_range, density=True)[0]

        # merge the weight table with the ntuple on binning columns
        weight_cols = _weight_cols
        if particle.column_names:
            weight_cols = particle.column_names
        binning_df = binning_df.merge(particle.merged_table[weight_cols + ['PDG', _fei_mode_col]],
                                      on=['PDG', _fei_mode_col], how='left')
        binning_df.index = ntuple_df.index
        particle.coverage = 1 - binning_df[weight_cols[0]].isna().sum() / len(binning_df)
        particle.plot_values = plot_values
        for col in weight_cols:
            ntuple_df[f'{particle.get_varname(col)}'] = binning_df[col]

    def reweight(self,
                 df: pd.DataFrame,
                 generate_variations: bool = True):
        """
        Reweights the dataframe according to the weight tables.

        Args:
            df (pandas.DataFrame): Dataframe containing the analysis ntuple.
            generate_variations (bool): When true generate weight variations.
        """
        for particle in self.particles:
            if particle.type not in _correction_types:
                raise ValueError(f'Particle type {particle.type} not supported!')
            print(f'Required variables: {self.get_ntuple_variables(df, particle)}')
            if generate_variations:
                particle.generate_variations(n_variations=self.n_variations)
            if particle.type == 'PID':
                self.add_pid_weight_columns(df, particle)
            elif particle.type == 'FEI':
                self.add_fei_weight_columns(df, particle)
        return df

    def print_coverage(self):
        """
        Prints the coverage of each particle.
        """
        print('Coverage:')
        for particle in self.particles:
            print(f'{particle.type} {particle.prefix.strip("_")}: {particle.coverage*100 :0.1f}%')

    def plot_coverage(self):
        """
        Plots the coverage of each particle.
        """
        for particle in self.particles:
            particle.plot_coverage()


def add_weights_to_dataframe(prefix: str,
                             df: pd.DataFrame,
                             systematic: str,
                             custom_tables: dict = None,
                             custom_thresholds: dict = None,
                             **kw_args) -> pd.DataFrame:
    """
    Helper method that adds weights to a dataframe.

    Args:
        prefix (str): Prefix for the new columns.
        df (pandas.DataFrame): Dataframe containing the analysis ntuple.
        systematic (str): Type  of the systematic corrections, options: "custom_PID" and "custom_FEI".
        MC_production (str): Name of the MC production.
        custom_tables (dict): Dictionary containing the custom efficiency weights.
        custom_thresholds (dict): Dictionary containing the custom thresholds for the  custom efficiency weights.
        n_variations (int): Number of variations to generate.
        generate_variations (bool): When true generate weight variations.
        weight_name (str): Name of the weight column.
        show_plots (bool): When true show the coverage plots.
        variable_aliases (dict): Dictionary containing variable aliases.
        cov_matrix (numpy.ndarray): Covariance matrix for the custom efficiency weights.
        fillna (int): Value to fill NaN values with.
        sys_seed (int): Seed for the systematic variations.
        syscorr (bool): When true assume systematics are 100% correlated defaults to true.
        **kw_args: Additional arguments for the Reweighter class.
    """
    generate_variations = kw_args.get('generate_variations', True)
    n_variations = kw_args.get('n_variations', 100)
    weight_name = kw_args.get('weight_name', "Weight")
    fillna = kw_args.get('fillna', 1.0)
    reweighter = Reweighter(n_variations=n_variations,
                            weight_name=weight_name,
                            fillna=fillna)
    variable_aliases = kw_args.get('variable_aliases')
    if systematic.lower() == 'custom_fei':
        cov_matrix = kw_args.get('cov_matrix')
        reweighter.add_fei_particle(prefix=prefix,
                                    table=custom_tables,
                                    threshold=custom_thresholds,
                                    variable_aliases=variable_aliases,
                                    cov=cov_matrix
                                    )
    elif systematic.lower() == 'custom_pid':
        sys_seed = kw_args.get('sys_seed')
        syscorr = kw_args.get('syscorr')
        if syscorr is None:
            syscorr = True
        reweighter.add_pid_particle(prefix=prefix,
                                    weights_dict=custom_tables,
                                    pdg_pid_variable_dict=custom_thresholds,
                                    variable_aliases=variable_aliases,
                                    sys_seed=sys_seed,
                                    syscorr=syscorr
                                    )
    else:
        raise ValueError(f'Systematic {systematic} is not supported!')

    result = reweighter.reweight(df, generate_variations=generate_variations)
    if kw_args.get('show_plots'):
        reweighter.print_coverage()
        reweighter.plot_coverage()
    return result
