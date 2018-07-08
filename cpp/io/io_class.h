#ifndef CPP_IO_IO_CLASS_H_
#define CPP_IO_IO_CLASS_H_

#include <string>

#include "cpp/core/global.h"
/**
 * an abstract io class, handling io for many locations
 */
class GraphIO {
 public:
  GraphIO() {}
  ~GraphIO() {}

  /**
   * open file location
   * @param location
   * @return true if success
   */
  virtual const bool OpenLocation(const std::string localtion) = 0;

  /**
   * get next line
   * @param line to fill
   * @return ture if not null
   */
  virtual const bool GetNextLine(std::string& line) = 0;
};

#endif //CPP_IO_IO_CLASS_H_