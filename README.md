# faiss-node
[![NPM Version](https://img.shields.io/npm/v/faiss-node?logo=npm)](https://www.npmjs.com/package/faiss-node)
[![Node Version](https://img.shields.io/node/v/faiss-node)](https://github.com/ewfian/faiss-node)
[![Unit Test](https://github.com/ewfian/faiss-node/actions/workflows/unit_test.yml/badge.svg)](https://github.com/ewfian/faiss-node/actions/workflows/unit_test.yml)
[![License](https://img.shields.io/github/license/ewfian/faiss-node)](https://github.com/ewfian/faiss-node)
[![Documentation](https://img.shields.io/badge/api-reference-blue.svg)](https://ewfian.github.io/faiss-node/)


faiss-node provides Node.js bindings for [faiss](https://github.com/facebookresearch/faiss)

_**This package is in a very early stage of development.**_


## Installation

```sh
$ npm install faiss-node
```

## Documentation

* [faiss-node API Documentation](https://ewfian.github.io/faiss-node/)

## Usage

```javascript
const { IndexFlatL2, Index, IndexFlatIP, MetricType } = require('faiss-node');

const dimension = 2;
const index = new IndexFlatL2(dimension);

console.log(index.getDimension()); // 2
console.log(index.isTrained()); // true
console.log(index.ntotal()); // 0

// inserting data into index.
index.add([1, 0]);
index.add([1, 2]);
index.add([1, 3]);
index.add([1, 1]);

console.log(index.ntotal()); // 4

const k = 4;
const results = index.search([1, 0], k);
console.log(results.labels); // [ 0, 3, 1, 2 ]
console.log(results.distances); // [ 0, 1, 4, 9 ]

// Save index
const fname = 'faiss.index';
index.write(fname);

// Load saved index
const index_loaded = IndexFlatL2.read(fname);
console.log(index_loaded.getDimension()); //2
console.log(index_loaded.ntotal()); //4
const results1 = index_loaded.search([1, 1], 4);
console.log(results1.labels); // [ 3, 0, 1, 2 ]
console.log(results1.distances); // [ 0, 1, 1, 4 ]

// Merge index
const newIndex = new IndexFlatL2(dimension);
newIndex.mergeFrom(index);
console.log(newIndex.ntotal()); // 4

// Remove items
console.log(newIndex.search([1, 2], 1)); // { distances: [ 0 ], labels: [ 1 ] }
const removedCount = newIndex.removeIds([0]);
console.log(removedCount); // 1
console.log(newIndex.ntotal()); // 3
console.log(newIndex.search([1, 2], 1)); // { distances: [ 0 ], labels: [ 0 ] }

// IndexFlatIP
const ipIndex = new IndexFlatIP(2);
ipIndex.add([1, 0]);

// Serialize an index
const index_buf = newIndex.toBuffer();
const deserializedIndex = Index.fromBuffer(index_buf);
console.log(deserializedIndex.ntotal()); // 3

// Factory index
const hnswIndex = Index.fromFactory(2, 'HNSW,Flat', MetricType.METRIC_INNER_PRODUCT);
const x = [1, 0, 0, 1];
hnswIndex.train(x);
hnswIndex.add(x);
```

## License

MIT