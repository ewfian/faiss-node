const { IndexFlatL2 } = require('../lib');

describe('IndexFlatL2', () => {
    describe('#read', () => {
        it('throws an error if file does not existed', () => {
            const fname = 'not_existed_file'
            expect(() => { IndexFlatL2.read(fname) }).toThrow(new RegExp(`^Error.*could not open ${fname} for reading: No such file or directory$`));
        });

        it('read saved file.', () => {
            const dimension = 2;
            const index = new IndexFlatL2(dimension);
            index.add([1, 0]);

            const fname = '_tmp.test.read.index';
            index.write(fname);

            const index_loaded = IndexFlatL2.read(fname);
            expect(index_loaded.getDimension()).toBe(2);
            expect(index_loaded.ntotal()).toBe(1);
        })
    });

    describe('#ntotal', () => {
        const index = new IndexFlatL2(1);

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
        const index = new IndexFlatL2(128);

        it('returns dimension', () => {
            expect(index.getDimension()).toBe(128);
        });
    });

    describe('#isTrained', () => {
        const index = new IndexFlatL2(1);

        it('returns true fixed', () => {
            expect(index.isTrained()).toBe(true);
        });
    });

    describe('#add', () => {
        const index = new IndexFlatL2(2);

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
        const index = new IndexFlatL2(2);

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
            expect(index.search([1, 0], 1)).toMatchObject({ distances: [0], labels: [0] });
            expect(index.search([1, 0], 4)).toMatchObject({ distances: [0, 1, 4, 9], labels: [0, 3, 1, 2] });
            expect(index.search([1, 1], 4)).toMatchObject({ distances: [0, 1, 1, 4], labels: [3, 0, 1, 2] });
        });
    });

    describe("#merge", () => {
        const index1 = new IndexFlatL2(2);
        beforeAll(() => {
            index1.add([1, 0]);
            index1.add([1, 2]);
            index1.add([1, 3]);
            index1.add([1, 1]);
        });

        const index2 = new IndexFlatL2(2);
        beforeAll(() => {
            index2.mergeFrom(index1);
        });

        it("throws an error if the number of arguments is not 1", () => {
            expect(() => { index2.mergeFrom() }).toThrow('Expected 1 argument, but got 0.');
            expect(() => { index2.mergeFrom(index1, 2) }).toThrow('Expected 1 argument, but got 2.');
        });

        it("throws an error if argument is not an object", () => {
            expect(() => { index2.mergeFrom(1) }).toThrow('Invalid argument type, must be an object.');
            expect(() => { index2.mergeFrom("string") }).toThrow('Invalid argument type, must be an object.');
        });

        it("throws an error if argument is not an instance of IndexFlatL2", () => {
            expect(() => { index2.mergeFrom({}) }).toThrow('Invalid argument');
            expect(() => { index2.mergeFrom({ "foo": "bar" }) }).toThrow('Invalid argument');
        });

        it("throws an error if merging index has different dimensions", () => {
            const index3 = new IndexFlatL2(3);
            expect(() => { index2.mergeFrom(index3) }).toThrow('The merging index must have the same dimension.');
        });

        it("returns search results on merged index", () => {
            expect(index2.search([1, 0], 1)).toMatchObject({
                distances: [0],
                labels: [0],
            });
            expect(index2.search([1, 0], 4)).toMatchObject({
                distances: [0, 1, 4, 9],
                labels: [0, 3, 1, 2],
            });
            expect(index2.search([1, 1], 4)).toMatchObject({
                distances: [0, 1, 1, 4],
                labels: [3, 0, 1, 2],
            });
        });
    });

    describe("#removeIds", () => {
        let index;
        beforeEach(() => {
            index = new IndexFlatL2(2);
            index.add([1, 0]);
            index.add([1, 1]);
            index.add([1, 2]);
            index.add([1, 3]);
        });

        it('throws an error if the count of given param is not 1', () => {
            expect(() => { index.removeIds() }).toThrow('Expected 1 argument, but got 0.');
            expect(() => { index.removeIds([], 1) }).toThrow('Expected 1 argument, but got 2.');
        });

        it('throws an error if given a non-Array object', () => {
            expect(() => { index.removeIds('[1, 2, 3]') }).toThrow('Invalid the first argument type, must be an Array.');
        });

        it('throws an error if the element of given array is not a number', () => {
            expect(() => { index.removeIds([1, '2']) }).toThrow('Expected a Number as array item. (at: 1)');
        });

        it("returns number of IDs removed", () => {
            expect(index.removeIds([])).toBe(0);
            expect(index.removeIds([0])).toBe(1);
            expect(index.removeIds([0, 1])).toBe(2);
            expect(index.removeIds([2])).toBe(0);
            expect(index.removeIds([0, 1, 2])).toBe(1);
        });

        it("correctly removed", () => {
            expect(index.search([1, 1], 1)).toMatchObject({ distances: [0], labels: [1] });
            expect(index.removeIds([0])).toBe(1);
            expect(index.search([1, 1], 1)).toMatchObject({ distances: [0], labels: [0] });
        });

        it("correctly removed multiple elements", () => {
            expect(index.search([1, 3], 1)).toMatchObject({ distances: [0], labels: [3] });
            expect(index.removeIds([0, 1])).toBe(2);
            expect(index.search([1, 3], 1)).toMatchObject({ distances: [0], labels: [1] });
        });

        it("correctly removed partal elements", () => {
            expect(index.search([1, 3], 1)).toMatchObject({ distances: [0], labels: [3] });
            expect(index.removeIds([0, 1, 2, 4, 5])).toBe(3);
            expect(index.search([1, 3], 1)).toMatchObject({ distances: [0], labels: [0] });
        });
    });
});