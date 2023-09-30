const { Index, MetricType } = require('../lib');

describe('Index', () => {
  describe('#fromFactory', () => {
    it('Flat', () => {
      const index = Index.fromFactory(2, 'Flat');
      const x = [1, 0, 0, 1];
      index.add(x);

      expect(index.ntotal()).toBe(2);
    });

    it('Flat /w IP', () => {
      const index = Index.fromFactory(2, 'Flat', MetricType.METRIC_INNER_PRODUCT);
      const x = [1, 0, 0, 1];
      index.add(x);

      expect(index.ntotal()).toBe(2);
    });
  });

  describe('#train', () => {
    it('HNSW training', () => {
      const index = Index.fromFactory(2, 'HNSW,Flat');
      const x = [1, 0, 0, 1];
      index.train(x);
      index.add(x);

      expect(index.ntotal()).toBe(2);
    });
  });


  describe('#toBuffer', () => {
    it('new index is same size as old', () => {
      const index = Index.fromFactory(2, 'Flat');
      const x = [1, 0, 0, 1];
      index.add(x);

      const buf = index.toBuffer();
      const newIndex = Index.fromBuffer(buf);

      expect(index.ntotal()).toBe(newIndex.ntotal());
    });
  });

  describe('#toIDMap2', () => {
    it('new index preserves ID\'s', () => {
      const index = Index.fromFactory(2, 'Flat').toIDMap2();
      const x = [1, 0, 0, 1];
      const labels = [100, 200];
      index.addWithIds(x, labels);
      const results = index.search([1, 0], 2);
      expect(results.labels).toEqual(labels);
    });

    it('supports BigInt labels', () => {
      const index = Index.fromFactory(2, 'Flat').toIDMap2();
      const x = [1, 0, 0, 1];
      const labels = [100n, 200n];
      index.addWithIds(x, labels);
      const results = index.search([1, 0], 2);
      expect(results.labels).toEqual([100, 200]);
      // TODO: Once search supports BigInt, use this test instead
      // expect(results.labels).toEqual(labels);
    });
  });
});
