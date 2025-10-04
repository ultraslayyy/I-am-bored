(function () {
    console.log("Anonymous, no args");
})();

(function (a, b) {
    console.log(a + b);
})(1, 2);

(function namedIIFE() {
    console.log("Named IIFE");
})();

(function namedArgsIIFE(x, y) {
    return x + y;
})(5, 7);

const iifeVar = (function () {
    console.log("Assigned anonymous IIFE");
})();

const iifeVarArgs = (function (x, y) {
    return x + y;
})(3, 4);

const namedIifeVar = (function namedAssignedIIFE() {
    return "hello";
})();

(() => {
    console.log("Arrow no args");
})();

((a, b) => a + b)(10, 20);

(function outer() {
    console.log("Outer");
    (function inner() {
        console.log("Inner");
    })();
})();

(function () {
    let x = Math.random();
    if (x > 0.5) return "high";
    return "low";
})();

(function () { return 42; })();

const multiStmtIIFE = (function () {
    let temp = 5;
    temp += 2;
    return temp * 2;
})();