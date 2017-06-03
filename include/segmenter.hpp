#ifndef SEGMENTER_HPP
#define SEGMENTER_HPP

#include <iostream>
#include <vector>
#include "protocol_headers.h"

class Segmenter {
private:
    std::vector<unsigned char[DATA_SIZE]> file_parts;
    std::vector<bool[DATA_SIZE]> sent_bytes;

public:
    Segmenter(char* file_name);
};


#endif
