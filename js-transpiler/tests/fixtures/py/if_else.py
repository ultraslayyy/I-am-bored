"/* MODULE F:\dead_drive\Repos\i-am-bored\tests\fixtures\if_else.js */"

from js_runtime.js_runtime import *

def isEven(n):
    if (n % 2) == 0:
        return True
    else:
        return False

console.log(isEven(4))

console.log(isEven(5))