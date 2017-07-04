import itertools

import datetime
from time import sleep

import os
import hashlib
import basf2

from git import Repo
from multiprocessing import Process

HEX_LENGTH = 10


class GitSnapshot:

    def __init__(self, repo_path):
        git_repo = Repo(repo_path)

        self._diff = git_repo.git.execute(["git", "diff", "HEAD"]).encode()
        self._diff_hash = hashlib.md5(self._diff).hexdigest()
        self._commit_hash = git_repo.head.commit.hexsha

    @property
    def git_hash(self):
        return self._commit_hash[:HEX_LENGTH] + "_" + self._diff_hash[:HEX_LENGTH]


class Environment:

    def __init__(self):
        self._path = None
        self._parameters = None
        self._input_file = None
        self._git_snapshot = None
        self._kwargs = None

    @property
    def parameters(self):
        if self._parameters is None:
            raise ValueError("You have to call set_parameters first!")
        return "/".join([str(key) + "=" + str(self._parameters[key]) for key in sorted(self._parameters)])

    @property
    def git_hash(self):
        return self._git_snapshot.git_hash

    @property
    def input_short_name(self):
        if not self._input_file:
            raise ValueError("You have to set the input_files first!")

        file_name = os.path.split(self._input_file)[1]
        short_file_name = os.path.splitext(file_name)[0]
        input_file_name_hash = hashlib.md5(self._input_file.encode()).hexdigest()[HEX_LENGTH:]
        return short_file_name + "_" + input_file_name_hash

    def set_parameters(self, parameters):
        self._parameters = parameters

    def set_path(self, path):
        self._path = path

    def set_kwargs(self, kwargs):
        self._kwargs = kwargs

    def get_output_folder(self):
        stub = "{env.git_hash}/{env.input_short_name}/{env.parameters}/"

        full_output_folder = os.path.join(os.getcwd(), stub.format(env=self))
        return full_output_folder

    def write_helpfile(self):
        with open("run_logging.dat", "w") as f:
            f.write(str(self) + "\n")

    def run(self):
        raise NotImplementedError

    def __str__(self):
        return_string = ""

        return_string += "Executed on: {date}".format(date=datetime.datetime.now()) + "\n"
        return_string += "With parameters:" + "\n"
        for key, value in self._parameters.items():
            return_string += "\t{key} = {value}".format(key=key, value=value) + "\n"

        return_string += "On git hash: {commit_hash}".format(commit_hash=self._git_snapshot._commit_hash) + "\n"
        return_string += "With additional diff: {diff}".format(diff=self._git_snapshot._diff) + "\n"
        return_string += "Using the input file: {input_file} {input_file_date}".format(
            input_file=self._input_file, input_file_date=os.path.getmtime(self._input_file)) + "\n"
        return_string += "Called with the arguments: {kwargs}".format(kwargs=self._kwargs) + "\n"

        return return_string


class Executor:

    def add_job(self, env):
        raise NotImplementedError

    def run(self):
        raise NotImplementedError


class LocalExecutor(Executor):

    def __init__(self, workers=None):
        self._workers = workers or os.cpu_count() // 2
        self._processes = []

    def start_job(self, env):
        self._processes.append(Process(target=LocalExecutor._execute_locally, args=(env,)))

    @staticmethod
    def _execute_locally(env):
        output_folder = env.get_output_folder()
        try:
            os.makedirs(output_folder)
        except FileExistsError:
            pass

        os.chdir(output_folder)
        env.write_helpfile()
        return env.run()

    def join(self):
        running_processes = []
        while self._processes:
            while len(running_processes) < self._workers and self._processes:
                next_process = self._processes.pop()
                next_process.start()
                running_processes.append(next_process)

            self.cleanup_process_list(running_processes)

        self.cleanup_process_list(running_processes, full_join=True)

    @staticmethod
    def cleanup_process_list(running_processes, full_join=False):
        for running_process in running_processes:
            if full_join:
                running_process.join()
                sleep(0.01)
            else:
                running_process.join(0.1)
            if running_process.exitcode is not None:
                del running_processes[running_processes.index(running_process)]


class Study:
    environment_type = None
    executor_type = None

    @classmethod
    def create_environment(cls):
        return cls.environment_type()

    @classmethod
    def create_executor(cls):
        return cls.executor_type()

    @classmethod
    def run(cls, create_path, **kwargs):
        output_folders = []

        parameter_names = kwargs.keys()
        parameter_values = kwargs.values()

        def add_parameter_name(values):
            return {parameter_name: value for parameter_name, value in zip(parameter_names, values)}

        every_parameter_combination = itertools.product(*parameter_values)
        every_parameter_combination_with_names = [add_parameter_name(values) for values in every_parameter_combination]

        pool = cls.create_executor()

        for parameters in every_parameter_combination_with_names:
            env = cls.create_environment()
            path_and_args = create_path(**parameters)

            if not path_and_args:
                continue

            if isinstance(path_and_args, tuple):
                path = path_and_args[0]
                path_kwargs = path_and_args[1]
            else:
                path = path_and_args
                path_kwargs = {}

            env.set_path(path)
            env.set_kwargs(path_kwargs)
            env.set_parameters(parameters)

            pool.start_job(env)

            output_folders.append(env.get_output_folder())

        pool.join()

        return output_folders


class Basf2GitSnapshot(GitSnapshot):

    def __init__(self):
        super().__init__(repo_path=os.getenv("BELLE2_LOCAL_DIR"))


class Basf2Environment(Environment):

    def __init__(self):
        super().__init__()
        self._git_snapshot = Basf2GitSnapshot()

    def run(self):
        if os.path.exists("result_ok"):
            print("Having already run with these settings.")
            return

        basf2.reset_log()
        basf2.log_to_file("log")
        basf2.process(self._path, **self._kwargs)

        with open("result_ok", "w") as f:
            f.write("OK\n")

    def set_path(self, path):
        super().set_path(path)

        input_modules = list(filter(lambda m: m.name() in ["RootInput", "SeqRootInput"], path.modules()))

        if len(input_modules) != 1:
            raise ValueError("Can not decide which module is the input module!")

        input_module = input_modules[0]
        input_files = list(filter(lambda param: param.name in ["inputFileName"], input_module.available_params()))

        if len(input_files) != 1:
            raise ValueError("Could not decide which parameter to use!")

        self._input_file = input_files[0].values


class Basf2Study(Study):
    executor_type = LocalExecutor
    environment_type = Basf2Environment
