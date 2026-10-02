#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import itertools
import unittest
import warnings

import numpy as np
import pandas as pd
from sysvar import Reweighter, add_weights_to_dataframe


class TestSysVar(unittest.TestCase):
    """Test case of SysVar module"""

    def __init__(self, *args, **kwargs):
        """Constructor"""
        super().__init__(*args, **kwargs)
        cosTheta_min = [-0.7, -0.5,   0, 0.2, 0.5, 0.8, 0.9]
        cosTheta_max = [-0.5,    0, 0.2, 0.5, 0.8, 0.9, 0.95]
        cosTheta_var = [list(c) for c in zip(cosTheta_min, cosTheta_max)]
        p_min = [0.5, 1, 2, 3, 4, 5]
        p_max = [1, 2, 3, 4, 5, 6]
        charge_var = [[-2, 0], [0, 2]]
        p_var = [list(p) for p in zip(p_min, p_max)]
        eff_binning = np.array([list(c) for c in itertools.product(p_var, cosTheta_var, charge_var)]).reshape(-1, 6)
        p_min = [0.5, 1, 2, 3, 4]
        p_max = [1, 2, 3, 4, 5]
        p_var = [list(p) for p in zip(p_min, p_max)]
        fake_binning = np.array([list(c) for c in itertools.product(p_var, cosTheta_var, charge_var)]).reshape(-1, 6)
        np.random.seed(0)
        eff_df = pd.DataFrame(eff_binning, columns=['p_min', 'p_max', 'cosTheta_min', 'cosTheta_max', 'charge_min', 'charge_max'])
        eff_df['data_MC_ratio'] = np.random.normal(1, 0.1, size=len(eff_df))
        eff_df['data_MC_uncertainty_stat_dn'] = np.random.uniform(0.001, 0.01, size=len(eff_df))
        eff_df['data_MC_uncertainty_stat_up'] = np.random.uniform(0.001, 0.01, size=len(eff_df))
        eff_df['data_MC_uncertainty_sys_up'] = np.random.uniform(0.001, 0.01, size=len(eff_df))
        eff_df['data_MC_uncertainty_sys_dn'] = np.random.uniform(0.001, 0.01, size=len(eff_df))
        eff_df['variable'] = 'electronID'
        eff_df['threshold'] = 0.8

        fake_df = pd.DataFrame(fake_binning, columns=['p_min', 'p_max', 'cosTheta_min', 'cosTheta_max', 'charge_min', 'charge_max'])
        fake_df['data_MC_ratio'] = np.random.normal(2, 0.2, size=len(fake_df))
        fake_df['data_MC_uncertainty_stat_dn'] = np.random.uniform(0.01, 0.02, size=len(fake_df))
        fake_df['data_MC_uncertainty_stat_up'] = np.random.uniform(0.01, 0.02, size=len(fake_df))
        fake_df['data_MC_uncertainty_sys_up'] = np.random.uniform(0.01, 0.02, size=len(fake_df))
        fake_df['data_MC_uncertainty_sys_dn'] = np.random.uniform(0.01, 0.02, size=len(fake_df))
        fake_df['variable'] = 'electronID'
        fake_df['threshold'] = 0.8
        #: PID tables for testing
        self.pid_tables = {(11, 11): eff_df, (11, 211): fake_df}

        user_data_dict = {
            'cosTheta': np.array([0.25, 0.55, 0.85, 0.925]),
            'p': np.array([1.5, 2.5, 3.5, 4.5]),
            'charge': np.array([-1, 1, 1, 1]),
            'electronID': np.array([0.85, 0.9, 0.95, 0.975]),
            'PDG': np.array([11, -11, -11, -11]),
            'mcPDG': np.array([11, 211, -11, 211]),
            'B0_dec_mode': [0, 1, 2, 3],
            'B0_PDG': [511, -511, 511, -511],
            'B0_sigProb': [0.02, 0.03, 0.02, 0.9]
        }
        #: User data for testing
        self.user_data = pd.DataFrame(user_data_dict)
        print(self.pid_tables)
        print(self.user_data)

        fei_dict = {'dec_mode': ['All', 'Rest', 'mode0', 'mode2'],
                    'cal': [0.8, 0.85, 0.75, 0.82],
                    'cal_stat_error': [0.1, 0.02, 0.03, 0.05],
                    'cal_sys_error': [0.1, 0.02, 0.03, 0.05],
                    'sig_prob_threshold': [0.01, 0.01, 0.01, 0.01],
                    'Btag': ['B0', 'B0', 'B0', 'B0']
                    }
        #: FEI tables for testing
        self.fei_table = pd.DataFrame(fei_dict)

    def test_merge_tables(self):
        """Tests merging of PID tables"""
        reweighter = Reweighter()
        thresholds = {11: ('electronID', 0.8)}
        merged_table = reweighter.merge_pid_weight_tables(self.pid_tables, thresholds)
        self.assertEqual(len(merged_table), 154)
        self.assertIn('PDG', merged_table.columns)
        self.assertIn('mcPDG', merged_table.columns)
        with self.assertRaises(ValueError):
            wrong_thresholds = {11: ('electronID', 0.9)}
            reweighter.merge_pid_weight_tables(self.pid_tables, wrong_thresholds)

    def test_binning(self):
        """Tests binning of PID tables"""
        reweighter = Reweighter()
        binning = reweighter.get_binning(self.pid_tables[(11, 11)])
        print(binning)
        self.assertEqual(len(binning), 3)
        self.assertCountEqual(binning.keys(), ['p', 'cosTheta', 'charge'])
        self.assertEqual(len(binning['p']), 7)
        self.assertEqual(len(binning['cosTheta']), 8)
        self.assertEqual(len(binning['charge']), 3)
        binning = reweighter.get_binning(self.pid_tables[(11, 211)])
        self.assertEqual(len(binning), 3)
        self.assertEqual(len(binning['p']), 6)

    def test_add_pid_particle(self):
        """Tests adding PID particle"""
        reweighter = Reweighter()
        thresholds = {11: ('electronID', 0.8)}
        reweighter.add_pid_particle('', self.pid_tables, thresholds)
        particle = reweighter.get_particle('')
        self.assertIsNotNone(particle)
        self.assertEqual(len(particle.merged_table), 154)
        self.assertCountEqual(particle.get_binning_variables(), ['p', 'cosTheta', 'charge'])
        print(particle)
        with self.assertRaises(ValueError):
            reweighter.add_pid_particle('', self.pid_tables, thresholds)

    def test_add_fei_particle(self):
        """Tests add_fei_particle method"""
        print('==================================================')
        reweighter = Reweighter()
        reweighter.add_fei_particle('', self.fei_table, 0.01, None)
        particle = reweighter.get_particle('')
        self.assertEqual(len(particle.merged_table), 4)
        self.assertCountEqual(particle.get_binning_variables(), ['dec_mode'])
        print(particle)
        with self.assertRaises(ValueError):
            reweighter.add_fei_particle('', self.fei_table, 0.001, None)

    def test_reweight(self):
        """Tests reweighting of PID and FEI particles"""
        n_variations = 50
        reweighter = Reweighter(n_variations=n_variations)
        thresholds = {11: ('electronID', 0.8)}
        reweighter.add_pid_particle('', self.pid_tables, thresholds, seed=42)
        reweighter.add_fei_particle('B0', self.fei_table, 0.01, None)
        local_data = self.user_data.copy(deep=True)
        reweighter.reweight(local_data)
        self.assertEqual(len(local_data), 4)
        self.assertIn('Weight', local_data.columns)
        self.assertIn('B0_Weight', local_data.columns)
        cols = [f'Weight_{i}' for i in range(0, n_variations)]
        self.assertCountEqual(cols, [col for col in local_data.columns if col.startswith('Weight_')])
        self.assertTrue((local_data.query('abs(mcPDG) == 11')[['Weight']+cols] < 1.5).all().all())
        self.assertTrue((local_data.query('abs(mcPDG) != 11')[['Weight']+cols] > 1.5).all().all())
        # print(local_data.query('abs(mcPDG) != 11')[['Weight']+cols].std(axis=1))
        self.assertTrue((local_data.query('abs(mcPDG) == 11')[['Weight']+cols].std(axis=1) < 0.015).all())
        self.assertTrue((local_data.query('abs(mcPDG) != 11')[['Weight']+cols].std(axis=1) > 0.015).all())

        cols = [f'B0_Weight_{i}' for i in range(0, n_variations)]
        self.assertCountEqual(cols, [col for col in local_data.columns if col.startswith('B0_Weight_')])
        # print(local_data[['B0_Weight']+cols])
        self.assertFalse((local_data[['B0_Weight']+cols].isna()).any().any())
        self.assertTrue((local_data[['B0_Weight']+cols] > 0).all().all())

        print('==================================================')
        print('==================================================')
        print('Shift index test:')
        local_data_shift = self.user_data.copy(deep=True)
        local_data_shift.index += 100
        reweighter.reweight(local_data_shift)
        self.assertIn('Weight', local_data_shift.columns)
        cols = [f'Weight_{i}' for i in range(0, n_variations)]
        print(local_data[['Weight']+cols])
        print(local_data_shift[['Weight']+cols])
        self.assertFalse((local_data_shift[['Weight']+cols].isna()).any().any())
        cols = [f'B0_Weight_{i}' for i in range(0, n_variations)]
        self.assertFalse((local_data_shift[['B0_Weight']+cols].isna()).any().any())

    def get_pid_weights(self, pid_tables, thresholds, df, **kwargs):
        """Reweights a copy of df with a single PID particle and returns the weight columns"""
        reweighter = Reweighter(n_variations=20)
        reweighter.add_pid_particle('', pid_tables, thresholds, **kwargs)
        local_data = df.copy(deep=True)
        reweighter.reweight(local_data)
        return local_data[reweighter.get_particle('').column_names]

    def get_variations(self, pid_tables, thresholds, **kwargs):
        """Returns the generated variations of a PID particle per bin of its weight table"""
        reweighter = Reweighter(n_variations=20)
        reweighter.add_pid_particle('', pid_tables, thresholds, **kwargs)
        particle = reweighter.get_particle('')
        particle.generate_variations(n_variations=20)
        return particle.merged_table[particle.column_names].values

    def relabelled_pid_tables(self):
        """Muon ID tables with the same binning and values as the electron ID tables"""
        tables = {}
        for (reco_pdg, mc_pdg), table in self.pid_tables.items():
            table = table.copy(deep=True)
            table['variable'] = 'muonID'
            tables[(13, 13 if mc_pdg == 11 else mc_pdg)] = table
        return tables

    def test_seed_reproducible(self):
        """Tests that seed makes the variations reproducible, also between different dataframes"""
        thresholds = {11: ('electronID', 0.8)}
        weights = self.get_pid_weights(self.pid_tables, thresholds, self.user_data, seed=42)
        weights_again = self.get_pid_weights(self.pid_tables, thresholds, self.user_data, seed=42)
        pd.testing.assert_frame_equal(weights, weights_again)
        # A different dataframe gets the same variations for the same bins
        subset = self.user_data.iloc[[1, 3]]
        weights_subset = self.get_pid_weights(self.pid_tables, thresholds, subset, seed=42)
        pd.testing.assert_frame_equal(weights.iloc[[1, 3]], weights_subset)
        # A different seed gives different variations, but the same nominal weight
        weights_other = self.get_pid_weights(self.pid_tables, thresholds, self.user_data, seed=43)
        pd.testing.assert_series_equal(weights['Weight'], weights_other['Weight'])
        self.assertFalse(np.allclose(weights.drop(columns='Weight'), weights_other.drop(columns='Weight')))

    def test_seed_stat_reproducible(self):
        """Tests that seed also fixes the statistical variations, unlike sys_seed"""
        thresholds = {11: ('electronID', 0.8)}
        # Remove the systematic uncertainties, so only the statistical variations remain
        stat_only = {key: table.assign(data_MC_uncertainty_sys_up=0., data_MC_uncertainty_sys_dn=0.)
                     for key, table in self.pid_tables.items()}
        np.testing.assert_array_equal(self.get_variations(stat_only, thresholds, seed=42),
                                      self.get_variations(stat_only, thresholds, seed=42))
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', UserWarning)
            self.assertFalse(np.allclose(self.get_variations(stat_only, thresholds, sys_seed=42),
                                         self.get_variations(stat_only, thresholds, sys_seed=42)))

    def test_seed_independent_tables(self):
        """Tests that tables with the same shape but different identity get independent variations"""
        electron = self.get_variations(self.pid_tables, {11: ('electronID', 0.8)}, seed=42)
        muon = self.get_variations(self.relabelled_pid_tables(), {13: ('muonID', 0.8)}, seed=42)
        self.assertEqual(electron.shape, muon.shape)
        self.assertFalse(np.allclose(electron, muon))
        # With sys_seed the systematic variations of both tables are identical
        no_stat = {key: table.assign(data_MC_uncertainty_stat_up=0., data_MC_uncertainty_stat_dn=0.)
                   for key, table in self.pid_tables.items()}
        no_stat_muon = {key: table.assign(data_MC_uncertainty_stat_up=0., data_MC_uncertainty_stat_dn=0.)
                        for key, table in self.relabelled_pid_tables().items()}
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', UserWarning)
            np.testing.assert_allclose(self.get_variations(no_stat, {11: ('electronID', 0.8)}, sys_seed=42),
                                       self.get_variations(no_stat_muon, {13: ('muonID', 0.8)}, sys_seed=42))
        # ... while with seed they are independent
        self.assertFalse(np.allclose(self.get_variations(no_stat, {11: ('electronID', 0.8)}, seed=42),
                                     self.get_variations(no_stat_muon, {13: ('muonID', 0.8)}, seed=42)))

    def test_seed_global_rng(self):
        """Tests that seed leaves the global numpy random state untouched"""
        np.random.seed(1)
        expected = np.random.uniform(size=5)
        np.random.seed(1)
        self.get_variations(self.pid_tables, {11: ('electronID', 0.8)}, seed=42)
        np.testing.assert_array_equal(np.random.uniform(size=5), expected)

    def test_seed_fei(self):
        """Tests the seed for FEI particles, with and without a covariance matrix"""
        cov = np.diag(np.full(4, 0.01))
        for cov_matrix in [None, cov]:
            results = [add_weights_to_dataframe('B0', self.user_data.copy(deep=True), 'custom_FEI',
                                                custom_tables=self.fei_table, custom_thresholds=0.01,
                                                n_variations=10, cov_matrix=cov_matrix, seed=seed)
                       for seed in [42, 42, 43]]
            cols = [f'B0_Weight_{i}' for i in range(10)]
            pd.testing.assert_frame_equal(results[0][cols], results[1][cols])
            self.assertFalse(np.allclose(results[0][cols], results[2][cols]))


if __name__ == '__main__':
    unittest.main()
