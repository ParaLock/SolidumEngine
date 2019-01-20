#pragma once

#include "Contract.h"
#include "IService.h"

namespace SolService {


    struct SchedulingStrategy {


    };

    class IScheduler {
    private:
    public:

        virtual void start() = 0;
        virtual void stop()  = 0;

        virtual void pause() = 0;

        virtual void addEntrypoint(ISolService* service, std::string entrypointName) = 0;

    };

}
