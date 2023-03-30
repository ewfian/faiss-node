export interface SearchResult {
    distances: number[],
    labels: number[]
}

export class IndexFlatL2 {
    constructor(dimension: number);
    ntotal(): number;
    getDimension(): number;
    isTrained(): boolean;
    add(x: number[]): void;
    search(x: number[], k: number): SearchResult;
}