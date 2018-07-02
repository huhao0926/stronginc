#ifndef CPP_ITERATOR_CONTAINER_H_
#define CPP_ITERATOR_CONTAINER_H_

template<class T>

/**
 * iterator container, store a pair of iterator
 */
class IterContainer {
 public:
  inline IterContainer(const T be, const T en): be_(be), en_(en) {}
  inline T begin() {return be_;}
  inline T end() {return en_;}

 private:
  T be_, en_;
};

#endif  // CPP_ITERATOR_CONTAINER_H_