#include "../include/RoundRobinScheduler.h"

using namespace SolService;

RoundRobinScheduler::RoundRobinScheduler() {

    m_isRunning = false;
}

void RoundRobinScheduler::start() {

    m_isRunning = true;

    while(m_isRunning) {

        for(int i = 0; i < thingsToDo.size(); i++) {

            thingsToDo[i]->invoke(nullptr, {});

        }

    }
}

void RoundRobinScheduler::stop() {

    m_isRunning = false;
}

void RoundRobinScheduler::pause() {

    m_isRunning = false;
}

void RoundRobinScheduler::addEntrypoint(ISolService* service, std::string entrypointName) {

    thingsToDo.push_back(service->getServiceContract().getFunction(entrypointName));

}