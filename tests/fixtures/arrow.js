(() => {
    console.log("Arrow no args");
});

((a, b) => a+b)(1, 2);

const namedArrow = (x, y) => x * y;
namedArrow(3, 4);

const anonArrow = () => console.log("Assigned anonymouse arrow");
anonArrow();

const arrowObj = () => ({ foo: 'bar' });
console.log(arrowObj());

const multiStmtArrow = (a, b) => {
    const sum = a + b;
    console.log('sum:', sum);
    return sum * 2;
}
multiStmtArrow(5, 7);

const arrowImplicit = n => n **2;
console.log(arrowImplicit(6));

[1, 2, 3].map(x => x + 10);

const outerArrow = x => y => x + y;
console.log(outerArrow(5)(10));

const arrowDefault = (a = 1, b = 2) => a + b;
console.log(arrowDefault(), arrowDefault(5));

const arrowRest = (...nums) => nums.reduce((a, b) => a + b, 0);
console.log(arrowRest(1, 2, 3, 4));

const arrowDestructure = (({ name, age }) => `${name} is ${age}`);
console.log(arrowDestructure({ name: 'Alice', age: 30 }));

(() => {
    let x = Math.random();
    if (x > 0.5) return 'high';
    return 'low';
})();

const arrowIIFE = () => (() => 42)();
console.log(arrowIIFE());

const multiStmtArrowIIFE = (() => {
    let temp = 5;
    temp += 2;
    return temp * 2;
})();
console.log(multiStmtArrowIIFE);