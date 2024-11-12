
import numpy as np
import lightgbm as lgb
import os
import tempfile


class State(object):
    """
    LGBM state
    """

    def __init__(self, bst=None, params=None, X_valid=None, y_valid=None, path='LGBM.txt', trainFraction=0.8, num_round=100):
        """ Constructor of the state object """
        #: X valid
        self.X_valid = X_valid
        #: Y valid
        self.y_valid = y_valid
        #: weights save path
        self.path = path
        #: LightGBM Model parameter
        self.params = params
        #: saved Best model
        self.bst = bst
        #: train fraction for dataset splitting
        self.trainFraction = trainFraction


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    # Create and return a state object containing the model and other necessary functions
    param = {'num_leaves': 31,
             'objective': 'regression',
             'learning_rate': 0.1,
             'device_type': "cpu",
             'deterministic': True,
             'metric': 'auc',
             'num_round': 100,
             'max_bin': 255,
             'boosting': "gbdt",
             }

    if isinstance(parameters, dict):
        param = {key: parameters[key] if key in param else value for key, value in param.items()}
    state = State(params=param, path=str(parameters['path']), trainFraction=float(
        parameters['trainFraction']))  # ,stop_round = int(parameters['stop_round']))
    return state

# Optimized begin_fit function (no specific initialization)


def begin_fit(state, Xtest, Stest, ytest, wtest, nBatches):
    # ignore origin splite, do it by your self
    return state


def feature_importance(state):
    """
    Return a list containing the feature importances
    """
    return state.bst.feature_importance('gain').tolist()


def partial_fit(state, X, S, y, w, epoch, batch):
    # Internal imply the batch and epoch setting, so just keep epoch and batch as default (1, and 0 )
    # Randomly shuffle the indices of the array
    # np.random.seed(42)  # Set seed for reproducibility
    shuffled_indices = np.random.permutation(X.shape[0])

    # Determine the split index
    split_index = int(X.shape[0] * state.trainFraction)

    state.train_set = lgb.Dataset(X[shuffled_indices[:split_index]], label=y[shuffled_indices[:split_index]])
    state.validation_set = state.train_set.create_valid(X[shuffled_indices[split_index:]], label=y[shuffled_indices[split_index:]])
    # callbacks=[lgb.early_stopping(stopping_rounds=state.stop_round)])
    state.bst = lgb.train(state.params, state.train_set, valid_sets=[state.validation_set])
    del shuffled_indices
    return True


def end_fit(state):
    # Save the best model state
    with tempfile.TemporaryDirectory() as path:
        state.bst.save_model(os.path.join(path, state.path))
        file_names = [state.path]
        files = []
        for file_name in file_names:
            with open(os.path.join(path, file_name), 'rb') as file:
                files.append(file.read())
        params = state.params
    del state
    return [file_names, files, params]


def load(obj):
    # Load the trained model
    with tempfile.TemporaryDirectory() as path:
        file_names = obj[0]
        for file_index, file_name in enumerate(file_names):
            with open(f'{path}/{file_name}', 'w+b') as file:
                file.write(bytes(obj[1][file_index]))

        bst = lgb.Booster(model_file=os.path.join(path, file_names[0]))  # init model
        state = State()
        state.bst = bst
        state.params = obj[2]
    return state


def apply(state, X):
    # Make predictions using the loaded model
    # X=state.StandardScaler(X)
    outputs = state.bst.predict(X)
    return np.require(outputs, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])
