const { IndexFlatIP } = require('../lib');

describe('IndexFlatIP', () => {
    describe('#constructor', () => {
        it('throws an error if the count of given param is not 1', () => {
            expect(() => { new IndexFlatIP() }).toThrow('Expected 1 argument, but got 0.');
            expect(() => { new IndexFlatIP(1, 2) }).toThrow('Expected 1 argument, but got 2.');
        });

        it('throws an error if given a non-Number object to the argument', () => {
            expect(() => { new IndexFlatIP('1') }).toThrow('Invalid the first argument type, must be a number.');
        });

        it('throws an error if functional call constructor', () => {
            expect(() => { IndexFlatIP(1) }).toThrow("Class constructors cannot be invoked without 'new'");
        });
    });

    describe('#ntotal', () => {
        const index = new IndexFlatIP(1);

        it('returns 0 if the index is just initialized', () => {
            expect(index.ntotal()).toBe(0);
        });

        it('returns total count', () => {
            index.add([1]);
            expect(index.ntotal()).toBe(1);
            index.add([1, 2, 3]);
            expect(index.ntotal()).toBe(4);
        });
    });

    describe('#getDimension', () => {
        const index = new IndexFlatIP(128);

        it('returns dimension', () => {
            expect(index.getDimension()).toBe(128);
        });
    });

    describe('#isTrained', () => {
        const index = new IndexFlatIP(1);

        it('returns true fixed', () => {
            expect(index.isTrained()).toBe(true);
        });
    });

    describe('#add', () => {
        const index = new IndexFlatIP(2);

        it('throws an error if the count of given param is not 1', () => {
            expect(() => { index.add() }).toThrow('Expected 1 argument, but got 0.');
            expect(() => { index.add([], 1) }).toThrow('Expected 1 argument, but got 2.');
        });

        it('throws an error if given a non-Array object', () => {
            expect(() => { index.add('[1, 2, 3]') }).toThrow('Invalid the first argument type, must be an Array.');
        });

        it('throws an error if the element of given array is not a number', () => {
            expect(() => { index.add([1, '2']) }).toThrow('Expected a Number as array item. (at: 1)');
        });

        it('throws an error if the length of given array is not adhere to the dimension of the index', () => {
            expect(() => { index.add([1, 2, 3]) }).toThrow('Invalid the given array length.');
            expect(() => { index.add([1, 2, 3, 4, 5]) }).toThrow('Invalid the given array length.');
            expect(() => { index.add([1]) }).toThrow('Invalid the given array length.');
        });
    });

    describe('#search', () => {
        const index = new IndexFlatIP(2);

        beforeAll(() => {
            index.add([1, 0]);
            index.add([1, 2]);
            index.add([1, 3]);
            index.add([1, 1]);
        });

        it('throws an error if the count of given param is not 2', () => {
            expect(() => { index.search() }).toThrow('Expected 2 arguments, but got 0.');
            expect(() => { index.search([], 1, 2) }).toThrow('Expected 2 arguments, but got 3.');
        });

        it('throws an error if given a non-Array object to first argument', () => {
            expect(() => { index.search('[1, 2, 3]', 2) }).toThrow('Invalid the first argument type, must be an Array.');
        });

        it('throws an error if given a non-Number object to second argument', () => {
            expect(() => { index.search([1, 2, 3], '2') }).toThrow('Invalid the second argument type, must be a Number.');
        });

        it('throws an error if given the number of neighborhoods exceeding the maximum number of elements', () => {
            expect(() => { index.search([1, 2], 6) }).toThrow('Invalid the number of k (cannot be given a value greater than `ntotal`: 4).');
        });

        it('throws an error if the element of given array is not a number', () => {
            expect(() => { index.search([1, '2'], 2) }).toThrow('Expected a Number as array item. (at: 1)');
        });

        it('throws an error if the length of given array is not adhere to the dimension of the index', () => {
            expect(() => { index.search([1, 2, 3], 2) }).toThrow('Invalid the given array length.');
            expect(() => { index.search([1, 2, 3, 4, 5], 2) }).toThrow('Invalid the given array length.');
            expect(() => { index.search([1], 2) }).toThrow('Invalid the given array length.');
        });

        it('returns search results', () => {
            expect(index.search([1, 0], 1)).toMatchObject({ distances: [1], labels: [0] });
            expect(index.search([1, 0], 4)).toMatchObject({ distances: [1, 1, 1, 1], labels: [3, 2, 1, 0] });
            expect(index.search([1, 1], 4)).toMatchObject({ distances: [4, 3, 2, 1], labels: [2, 1, 3, 0] });
        });
    });
});