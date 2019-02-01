#pragma once

#include "../../../SolidumAPI/include/ResourceAPI.h"
#include "../../../SolidumAPI/include/EngineAPI.h"
#include "../../../SolidumAPI/include/ServiceAPI.h"

#include "IScheduler.h"
#include "IService.h"

#include "Contract.h"

namespace SolService {

    class RoundRobinScheduler : public IScheduler {
    private:

        std::vector<ISolFunction*> thingsToDo;

        bool m_isRunning;

    public:

        RoundRobinScheduler();

        void start();
        void stop();
        void pause();

        void addEntrypoint(ISolService* service, std::string entrypointName);
    };
}

