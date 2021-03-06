#include "Assembler.hpp"

bool compareFunc(const fc_header& pck1, const fc_header& pck2) {
    return pck1.frame_count < pck2.frame_count;
}

Assembler::Assembler(char* fileName) {
    this->fileName = fileName;
    this->expected = 0;
    this->fileParts.clear();
	this->finished = false;

	fh = fopen(fileName, "wb");
	if(fh == NULL) {
		std::cout << "[ERROR] Couldn't open a file: " << fileName << std::endl;
	}
}


void Assembler::pushToBuffer(fc_header pck) {
    if(pck.frame_count < expected) {
        return;
    }

    std::unique_lock<std::mutex> lock(listMutex);
    fileParts.push_back(pck);

	fileParts.sort(compareFunc);
}

void Assembler::writeToFile() {

    std::unique_lock<std::mutex> lock(listMutex);
    if(!fileParts.empty()) {
        fc_header pck = (fc_header)fileParts.front();

        while(pck.frame_count == expected) {
            fileParts.pop_front();

			//Write binary to file
			fwrite(pck.data, sizeof(char), pck.data_len, fh);

            printf("Downloading %.2f%% \r", (float)expected / pck.num_of_total_frames * 100);
            fflush(stdout);

            expected++;

			if(pck.frame_count == pck.num_of_total_frames - 1) {
                finished = true;
            }

            if(!fileParts.empty())
                pck = (fc_header)fileParts.front();
            else
                break;
        }
    }
}

void Assembler::printBuffer() {
    std::list<fc_header>::iterator it;

    std::unique_lock<std::mutex> lock(listMutex);
    if(fileParts.empty()) {
        std::cout<< "Buffer is empty" <<std::endl;
        return;
    }

    for(it = fileParts.begin(); it != fileParts.end(); it++) {
        fc_header temp = (fc_header)(*it);

        std::cout<< temp.frame_count <<std::endl;
    }
}
