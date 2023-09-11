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
});
