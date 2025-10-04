"/* MODULE F:\dead_drive\Repos\i-am-bored\tests\fixtures\fib.js */"

from js_runtime.js_runtime import *

def fib(n):
    if n <= 1:
        return n
    return (fib((n - 1)) + fib((n - 2)))

console.log(fib(6))