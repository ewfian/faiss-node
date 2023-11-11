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

## Setup for contibutors

This section is only relevant for you, in case you'd like to contribute to this project:

```sh
# install JS/node dependencies
npm i

# in case you forked, deps/faiss submodule wouldn't by default; 
# checkout the FAISS source submodule
git submodule update --init --recursive

# compile the binding code
npm run build:debug

# test
npm run test
```

### Pitfall: M1/M2 OpenMP linking architecture mismatch 

In case you see this warning:

> ld: warning: ignoring file /usr/local/opt/libomp/lib/libomp.dylib, building for macOS-arm64 but attempting to link with file built for macOS-x86_64

...or testing the library errors out with:

`dlopen(faiss-node/build/Debug/faiss-node.node, 0x0001): symbol not found in flat namespace (___kmpc_barrier)``

You're probably running on an machine with a M1/M2 chip and running into a multi-architecture linking issue.
 `node-faiss` must be linked against `OpenMP`, but the binary architecute of the compiled library and the
 architecture of `OpenMP` library must match. You'll run into this issue if one of the architectures differ:

```zsh
user@dev faiss-node % arch
arm64
user@dev % file /usr/local/opt/libomp/lib/libomp.dylib
/usr/local/opt/libomp/lib/libomp.dylib: Mach-O 64-bit dynamically linked shared library x86_64
user@dev % 
```

You might need to reinstall `Homebrew` for M1/M2 and install `libomp` again from a host terminal
process that is running in `arm64` architecture or explicity request for `arm64` when installing OpenMP: 
`arch -arm64 brew install libomp`.

In case linking still ends up with the same error, try:
```zsh
export LDFLAGS="-L/opt/homebrew/opt/libomp/lib"
export CPPFLAGS="-I/opt/homebrew/opt/libomp/include"
```

Run `npm run build` again and make sure that `libomp.dylib` is picked from `/opt/homebrew` (arm64) folder.

## License

MIT