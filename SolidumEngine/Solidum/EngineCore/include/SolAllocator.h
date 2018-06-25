#pragma once

#include "../../../SolidumAPI/include/EngineAPI.h"

#include <stdlib.h>

//Yup this memory allocator sucks... Will improve later. Actually... Will implement later.
class SolAllocator : public ISolAllocator {
private:
public:
	void* getMemory(size_t size);
	void  freeMemory(size_t size, void*);
};
