#include "../include/Bus.h"

using namespace SolService;

void Bus::attachService(ISolService* service) {
    
    m_manager.addService(service);
}

IAllocator& Bus::getAllocator() {
    return m_allocator;
}

IScheduler& Bus::getServiceScheduler() {
    return m_scheduler;
}

IEngine& Bus::getEngine() {
    return m_engine;
}