/** Searh result object. */
export interface SearchResult {
    /** The disances of the nearest negihbors found, size n*k. */
    distances: number[],
    /** The labels of the nearest neighbors found, size n*k. */
    labels: number[]
}

// See faiss/MetricType.h
export enum MetricType {
    METRIC_INNER_PRODUCT = 0, ///< maximum inner product search
    METRIC_L2 = 1,            ///< squared L2 search
    METRIC_L1,                ///< L1 (aka cityblock)
    METRIC_Linf,              ///< infinity distance
    METRIC_Lp,                ///< L_p distance, p is given by a faiss::Index
    /// metric_arg

    /// some additional metrics defined in scipy.spatial.distance
    METRIC_Canberra = 20,
    METRIC_BrayCurtis,
    METRIC_JensenShannon,
    METRIC_Jaccard, ///< defined as: sum_i(min(a_i, b_i)) / sum_i(max(a_i, b_i))
    ///< where a_i, b_i > 0
}

/**
 * Index.
 * Index that stores the full vectors and performs exhaustive search.
 * @param {number} d The dimensionality of index.
 */
export class Index {
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
     * Train n vectors of dimension d to the index.
     * Vectors are implicitly assigned labels ntotal .. ntotal + n - 1
     * @param {number[]} x Input matrix, size n * d
     */
    train(x: number[]): void;
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
    write(fname: string): void;
    /** 
     * Write index to buffer.
     */
    toBuffer(): Buffer;
    /** 
     * Read index from a file.
     * @param {string} fname File path to read.
     * @return {Index} The index read.
     */
    static read(fname: string): Index;
    /** 
     * Read index from buffer.
     * @param {Buffer} src Buffer to create index from.
     * @return {Index} The index read.
     */
    static fromBuffer(src: Buffer): Index;
    /** 
     * Construct an index from factory descriptor.
     * @param {number} dims Buffer to create index from.
     * @param {string} descriptor Factory descriptor.
     * @param {MetricType} metric Metric type (defaults to L2).
     * @return {Index} The index read.
     */
    static fromFactory(dims: number, descriptor: string, metric?: MetricType): Index;
    /**
     * Merge the current index with another Index instance.
     * @param {Index} otherIndex The other Index instance to merge from.
     */
    mergeFrom(otherIndex: Index): void;
    /**
     * Remove IDs from the index.
     * @param {number[]} ids IDs to read.
     * @return {number} number of IDs removed.
     */
    removeIds(ids: number[]): number

}

/**
 * IndexFlatL2 Index.
 * IndexFlatL2 that stores the full vectors and performs `squared L2` search.
 * @param {number} d The dimensionality of index.
 */
export class IndexFlatL2 extends Index {
    /** 
     * Read index from a file.
     * @param {string} fname File path to read.
     * @return {IndexFlatL2} The index read.
     */
    static read(fname: string): IndexFlatL2;
    /** 
     * Read index from buffer.
     * @param {Buffer} src Buffer to create index from.
     * @return {IndexFlatL2} The index read.
     */
    static fromBuffer(src: Buffer): IndexFlatL2;
    /**
     * Merge the current index with another IndexFlatL2 instance.
     * @param {IndexFlatL2} otherIndex The other IndexFlatL2 instance to merge from.
     */
    mergeFrom(otherIndex: IndexFlatL2): void;
}

/**
 * IndexFlatIP Index.
 * Index that stores the full vectors and performs `maximum inner product` search.
 * @param {number} d The dimensionality of index.
 */
export class IndexFlatIP extends Index {
    /** 
     * Read index from a file.
     * @param {string} fname File path to read.
     * @return {IndexFlatIP} The index read.
     */
    static read(fname: string): IndexFlatIP;
    /** 
     * Read index from buffer.
     * @param {Buffer} src Buffer to create index from.
     * @return {IndexFlatIP} The index read.
     */
    static fromBuffer(src: Buffer): IndexFlatIP;
    /**
     * Merge the current index with another IndexFlatIP instance.
     * @param {IndexFlatIP} otherIndex The other IndexFlatIP instance to merge from.
     */
    mergeFrom(otherIndex: IndexFlatIP): void;
}