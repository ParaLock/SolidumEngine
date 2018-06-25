#include "../include/SolAllocator.h"

void * SolAllocator::getMemory(size_t size)
{
	return malloc(size);
}

void SolAllocator::freeMemory(size_t size, void * ptr)
{
	delete ptr;
}
