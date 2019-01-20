#pragma once

namespace SolService {

    class IScheduler;

    class IBus {
    public:
        virtual IScheduler& getServiceScheduler() = 0;
        virtual IAllocator& getAllocator()        = 0;
        virtual IEngine&    getEngine()           = 0;

        virtual void        attachService(ISolService* service)       = 0;
    };

}
