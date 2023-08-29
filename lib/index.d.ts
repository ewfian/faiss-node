/** Searh result object. */
export interface SearchResult {
    /** The disances of the nearest negihbors found, size n*k. */
    distances: number[],
    /** The labels of the nearest neighbors found, size n*k. */
    labels: number[]
}

/**
 * IndexFlatL2 Index.
 * Index that stores the full vectors and performs exhaustive search.
 * @param {number} d The dimensionality of index.
 */
export class IndexFlatL2 {
    constructor(d: number);
    /**
     * returns the number of verctors currently indexed.
     * @return {numbers} The number of verctors currently indexed.
     */
    ntotal(): number;
    /**
     * returns the dimensionality of verctors.
     * @return {number} The dimensionality of verctors.
     */
    getDimension(): number;
    /**
     * returns a boolean that indicates whether training is required.
     * @return {number} Whether training is required.
     */
    isTrained(): boolean;
    /** 
     * Add n vectors of dimension d to the index.
     * Vectors are implicitly assigned labels ntotal .. ntotal + n - 1
     * @param {number[]} x Input matrix, size n * d
     */
    add(x: number[]): void;
    /** 
     * Query n vectors of dimension d to the index.
     * return at most k vectors. If there are not enough results for a
     * query, the result array is padded with -1s.
     *
     * @param {number[]} x Input vectors to search, size n * d.
     * @param {number} k The number of nearest neighbors to search for.
     * @return {SearchResult} Output of the search result.
     */
    search(x: number[], k: number): SearchResult;
    /** 
     * Write index to a file.
     * @param {string} fname File path to write.
     */
    write(fname: string): void
    /** 
     * Read index from a file.
     * @param {string} fname File path to read.
     * @return {IndexFlatL2} The index read.
     */
    static read(fname: string): IndexFlatL2;
    /**
     * Merge the current index with another IndexFlatL2 instance.
     * @param {IndexFlatL2} otherIndex The other IndexFlatL2 instance to merge from.
     */
    mergeFrom(otherIndex: IndexFlatL2): void;
    /**
     * Remove IDs from the index.
     * @param {string} ids IDs to read.
     * @return {IndexFlatL2} number of IDs removed.
     */
    removeIds(ids: number[]): number
}