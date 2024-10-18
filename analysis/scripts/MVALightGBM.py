
import numpy as np
import lightgbm as lgb
import os
import tempfile


class StandardScaler():
    """Standardize data by removing the mean and scaling to
    unit variance. This object can be used as a transform
    in PyTorch data loaders.

    Args:
        mean (numpy.ndarray): The mean value for each feature in the data.
        scale (numpy.ndarray): Per-feature relative scaling.
    """

    def __init__(self, mean=None, scale=None):
        self.mean_ = mean
        self.scale_ = scale

    def fit(self, sample):
        """Set the mean and scale values based on the sample data.
        """
        self.mean_ = np.mean(sample, axis=0, keepdims=True)
        self.scale_ = np.std(sample, axis=0, ddof=0, keepdims=True)
        return self

    def __call__(self, sample):
        return (sample - self.mean_) / self.scale_

    def inverse_transform(self, sample):
        """Scale the data back to the original representation
        """
        return sample * self.scale_ + self.mean_


class State(object):
    """
    Tensorflow state
    """

    def __init__(self, bst=None, params=None, X_valid=None, y_valid=None, path='LGBM.txt', trainFraction=0.8, num_round=100):
        """ Constructor of the state object """
        #: tensorflow model inheriting from tf.Module
        self.X_valid = X_valid
        self.y_valid = y_valid
        self.path = path
        self.params = params
        self.scale_param = {}
        self.bst = bst
        self.StandardScaler = StandardScaler()
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
    print(f"0: {X.shape[0]}, 1: {X.shape[1]}, len: {len(X)}")
    # Randomly shuffle the indices of the array
    np.random.seed(42)  # Set seed for reproducibility
    shuffled_indices = np.random.permutation(X.shape[0])

    # Determine the split index
    split_index = int(X.shape[0] * state.trainFraction)

    DoScaler = False
    if DoScaler:
        state.StandardScaler.fit(X)
        X = state.StandardScaler(X)
        state.scale_param['mean'] = state.StandardScaler.mean_
        state.scale_param['scale'] = state.StandardScaler.scale_

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
        scale = state.scale_param
        print(scale)
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
        # state.StandardScaler.__init__(mean=obj[3]['mean'],scale=obj[3]['scale'])
    return state


def apply(state, X):
    # Make predictions using the loaded model
    # X=state.StandardScaler(X)
    outputs = state.bst.predict(X)
    return np.require(outputs, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])
