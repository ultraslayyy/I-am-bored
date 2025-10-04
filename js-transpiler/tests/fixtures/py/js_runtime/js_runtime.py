import logging
import os
import random
import sys
import time
import warnings

class ArrayClass:
    def isArray(self, arg):
        isinstance(arg, list)
Array = ArrayClass()

class Console:
    def clear(self):
        if os.name == 'nt':
            os.system('cls')
        else:
            os.system('clear')
    def dir(self, arg):
        print(dir(arg))
    def error(self, *args):
        print(*args, file=sys.stderr)
    def info(self, *args):
        info_string = ' '.join(args)
        logging.basicConfig(level=logging.INFO)
        logging.info(info_string)
    def log(self, *args):
        print(*args)
    def warn(self, *args):
        warn_string = ' '.join(args)
        warnings.warn(warn_string, UserWarning)
console = Console()

class Date:
    def now(self):
        time.time()

class Math:
    def random(self):
        random()
math = Math();