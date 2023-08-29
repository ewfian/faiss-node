const { IndexFlatL2 } = require('../');

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

const k = 4;
const results = index.search([1, 0], k);
console.log(results.labels);
console.log(results.distances);

const fname = 'faiss.index';
index.write(fname);

const index_loaded = IndexFlatL2.read(fname);
console.log(index_loaded.getDimension());
console.log(index_loaded.ntotal());
const results1 = index_loaded.search([1, 1], 4);
console.log(results1.labels);
console.log(results1.distances);

const newIndex = new IndexFlatL2(dimension);
newIndex.mergeFrom(index);
console.log(newIndex.ntotal());

console.log(newIndex.search([1, 2], 1));
const removedCount = newIndex.removeIds([0]);
console.log(removedCount);
console.log(newIndex.ntotal());
console.log(newIndex.search([1, 2], 1));