# faiss-node
This package is in a very early stage of development.

## Installation

```sh
$ npm install faiss-node
```

## Usage

```typescript
import { IndexFlatL2 } from 'faiss-node';

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

const results = index.search([1, 0], 4);
console.log(results.labels); // [ 0, 3, 1, 2 ]
console.log(results.distances); // [ 0, 1, 4, 9 ]
```