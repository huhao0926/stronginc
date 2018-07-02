#ifndef CPP_IO_IO_LOCAL_H_
#define CPP_IO_IO_LOCAL_H_

#include <stdio.h>
#include <iostream>
#include <string>

#include "cpp/io/io_class.h"

#define LINESIZE 0xffffff

/**
 * subclass of io_class
 * used by local
 */
class GraphIOLocal: public GraphIO {
 public:
  GraphIOLocal(): GraphIO() {}

  ~GraphIOLocal() {
    if (file != nullptr) {
      fclose(file);
      file = nullptr;
    }
  }

  inline const bool OpenLocation(const std::string location) {
    file = fopen(location.c_str(), "r");
    if (file == nullptr) {
      LOG(FATAL) << "File doesn't exist: " << location << std::endl;
      return false;
    }
    return true;
  }

  inline const bool GetNextLine(std::string& line) {
    if (fgets(buff, LINESIZE, file)) {
      std::string str(buff);
      line = str;
      return true;
    } else {
      fclose(file);
      file = nullptr;
    }
    return false;
  }

 private:
  FILE* file;
  char buff[LINESIZE];
};

#endif //CPP_IO_IO_LOCAL_H_