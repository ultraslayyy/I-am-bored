"/* MODULE F:\dead_drive\Repos\i-am-bored\tests\fixtures\iife.js */"

do ->
  console.log("Anonymous, no args")

do ->
  console.log(a + b)

do ->
  console.log("Named IIFE")

do ->
  return x + y

iifeVar = do ->
  console.log("Assigned anonymous IIFE")

iifeVarArgs = do ->
  return x + y

namedIifeVar = do ->
  return "hello"

()

(10, 20)

do ->
  console.log("Outer")
  do ->
    console.log("Inner")

do ->
  x = Math.random()
  if x > 0.5
    return "high"
  return "low"

do ->
  return 42

multiStmtIIFE = do ->
  temp = 5
  null
  return temp * 2