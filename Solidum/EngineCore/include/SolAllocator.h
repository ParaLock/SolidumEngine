#pragma once

#include "EngineAPI.h"
#include <stdlib.h>

//Yup this memory allocator sucks... Will improve later. Actually... Will implement later.
class SolAllocator : public IAllocator {
private:
public:

	SolAllocator() {

	}

	~SolAllocator() {

	}

	void* getMemory(size_t size) override {
		return malloc(size);
	}
	void  freeMemory(size_t size, void* mem) override {
		free(mem);
	}
};
