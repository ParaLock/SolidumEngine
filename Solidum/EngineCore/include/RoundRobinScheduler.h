#pragma once

#include "ResourceAPI.h"
#include "EngineAPI.h"
#include "ServiceAPI.h"

#include "IScheduler.h"
#include "IService.h"

#include "Contract.h"

namespace SolService {

    class RoundRobinScheduler : public IScheduler {
    private:

        std::vector<ISolFunction*> thingsToDo;

        bool m_isRunning;

    public:

        RoundRobinScheduler() {

            m_isRunning = false;
        }

        void start() {

            m_isRunning = true;

            while(m_isRunning) {

                for(int i = 0; i < thingsToDo.size(); i++) {

                    thingsToDo[i]->invoke(nullptr, {});

                }

            }
        }

        void stop() {

            m_isRunning = false;
        }

        void pause() {

            m_isRunning = false;
        }

        void addEntrypoint(ISolService* service, std::string entrypointName) {

            thingsToDo.push_back(service->getServiceContract().getFunction(entrypointName));

        }


    };


}

