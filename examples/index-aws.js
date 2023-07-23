const { IndexFlatL2 } = require('faiss-node');

exports.handler = async function(event, context) {
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

    return results;
};
