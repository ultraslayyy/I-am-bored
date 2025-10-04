"/* MODULE F:\dead_drive\Repos\i-am-bored\tests\fixtures\iife.js */"

from js_runtime.js_runtime import *

def _anon_iife():
    console.log("Anonymous, no args")
_anon_iife()

def _anon_iife(a, b):
    console.log((a + b))
_anon_iife(1, 2)

def namedIIFE():
    console.log("Named IIFE")
namedIIFE()

def namedArgsIIFE(x, y):
    return (x + y)
namedArgsIIFE(5, 7)

def _anon_iife():
    console.log("Assigned anonymous IIFE")
iifeVar = _anon_iife()

def _anon_iife(x, y):
    return (x + y)
iifeVarArgs = _anon_iife(3, 4)

def namedAssignedIIFE():
    return "hello"
namedIifeVar = namedAssignedIIFE()

def _anon_iife():
    console.log("Arrow no args")
_anon_iife()()

(lambda a, b: (a + b))(10, 20)(10, 20)

def outer():
    console.log("Outer")
    def inner():
        console.log("Inner")
    inner()
outer()

def _anon_iife():
    x = Math.random()
    if x > 0.5:
        return "high"
    return "low"
_anon_iife()

(lambda: 42)()

def _anon_iife():
    temp = 5
    None
    return (temp * 2)
multiStmtIIFE = _anon_iife()