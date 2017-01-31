import signal
import os
import sys


def sigquithandler(sig, frame):
    print('SIGQUIT handler called')
    print("dummy abort")
    sys.exit(0)

# TODO investigating test failure on bamboo. (fails to die on SIGQUIT in single process.)
prevsig = signal.signal(signal.SIGQUIT, sigquithandler)
print('handler installed, previous handler was: ', prevsig)
pid = os.getpid()
print("Killing %s in init (sig %d)" % (pid, signal.SIGQUIT))
os.kill(pid, signal.SIGQUIT)

# this wouldn't change a thing, right?
# os.kill(pid, 3)

# if handler is called we never end up here.
sys.exit(1)
