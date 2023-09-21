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

  describe('#reset', () => {
    let index;

    beforeEach(() => {
      index = Index.fromFactory(2, 'Flat');
      index.add([1, 0, 0, 1]);
    });

    it('reset the index', () => {
      expect(index.ntotal()).toBe(2);
      index.reset();
      expect(index.ntotal()).toBe(0);
    });

    it('reset the index and add new elements', () => {
      expect(index.ntotal()).toBe(2);
      index.reset();
      expect(index.ntotal()).toBe(0);

      index.add([1, 0]);
      index.add([1, 2]);
      expect(index.ntotal()).toBe(2);
    });
  });

  describe('#dispose', () => {
    let index;

    beforeEach(() => {
      index = Index.fromFactory(2, 'Flat');
      index.add([1, 0, 0, 1]);
    });

    it('disposing an index does not throw', () => {
      index.dispose();
    });

    it('disposing twice will throw', () => {
      index.dispose();

      expect(() => index.dispose()).toThrow('Index has been disposed');
    });

    it('invoking a function after dispose will throw', () => {
      index.dispose();

      expect(() => index.ntotal()).toThrow('Index has been disposed');
    });
  });
});
