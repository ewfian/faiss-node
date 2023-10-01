const { IndexHNSW, MetricType } = require('../lib');

describe('IndexHNSW', () => {
  describe('#constructor', () => {
    it('1 arg will result in index with default neighbors & metric', () => {
      const index = new IndexHNSW(2);
      expect(index.getDimension()).toBe(2);
    });

    it('2 args will result in index with default metric', () => {
      const index = new IndexHNSW(2, 20);
      expect(index.getDimension()).toBe(2);
    });

    it('3 args will result in index', () => {
      const index = new IndexHNSW(2, 20, MetricType.METRIC_INNER_PRODUCT);
      expect(index.getDimension()).toBe(2);
    });
  });
});
