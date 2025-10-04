"/* MODULE F:\dead_drive\Repos\i-am-bored\tests\fixtures\arrow.js */"

from js_runtime.js_runtime import *

def _anon_arrow():
    console.log("Arrow no args")

(lambda a, b: (a + b))(1, 2)(1, 2)

namedArrow = lambda x, y: (x * y)

namedArrow(3, 4)

anonArrow = lambda : console.log("Assigned anonymouse arrow")

anonArrow()

arrowObj = lambda : None

console.log(arrowObj())

def _anon_multiStmtArrow(a, b):
    sum = (a + b)
    console.log("sum:", sum)
    return (sum * 2)
multiStmtArrow = _anon_multiStmtArrow

multiStmtArrow(5, 7)

arrowImplicit = lambda n: (n ** 2)

console.log(arrowImplicit(6))

null.map((lambda x: (x + 10)))

outerArrow = lambda x: (lambda y: (x + y))

console.log(outerArrow(5)(10))

arrowDefault = lambda a=1, b=2: (a + b)

console.log(arrowDefault(), arrowDefault(5))

arrowRest = lambda *nums: nums.reduce((lambda a, b: (a + b)), 0)

console.log(arrowRest(1, 2, 3, 4))

arrowDestructure = lambda _arg: None

console.log(arrowDestructure(None))

def _anon_iife():
    x = Math.random()
    if x > 0.5:
        return "high"
    return "low"
_anon_iife()()

arrowIIFE = lambda : (lambda : 42)()()

console.log(arrowIIFE())

def _anon_iife():
    temp = 5
    None
    return (temp * 2)
_anon_iife()
multiStmtArrowIIFE = _anon_multiStmtArrowIIFE()

console.log(multiStmtArrowIIFE)