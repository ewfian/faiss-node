const assert = require('assert');
const { IndexFlatL2 } = require('faiss-node');

const dimension = 4096;
const index = new IndexFlatL2(dimension);
assert.equal(index.getDimension(), dimension);

console.log(process.version);
console.log(process.platform);
console.log(process.arch);
console.log(index.getDimension());
