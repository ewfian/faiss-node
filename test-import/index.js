const assert = require('assert');
const { IndexFlatL2 } = require('../lib/index');

const dimension = 4096;
const index = new IndexFlatL2(dimension);
assert.equal(index.getDimension(), dimension)
