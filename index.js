const { IndexFlatL2 } = require('./lib/index');

const dimension = 2;
const index = new IndexFlatL2(dimension);

console.log(index.getDimension());
console.log(index.isTrained());
console.log(index.ntotal());
index.add([1, 0]);
index.add([1, 2]);
index.add([1, 3]);
index.add([1, 1]);
console.log(index.ntotal());

const results = index.search([1, 0], 4);
console.log(results.labels);
console.log(results.distances);
