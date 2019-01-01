#pragma once

#include "ObjectPool.h"

#include "Bus.h"
#include "IScheduler.h"

#include "EngineAPI.h"
#include "ServiceAPI.h"
#include "ResourceAPI.h"

#include "IService.h"

#include "Contract.h"

using namespace ObjectPool;

namespace SolService {

	struct Request {

		Request() {
		}

		bool           isAsync;

		ObjectID       client;
		std::string    callName;
		ObjectID       argBufferID;
	
		SolAny*        retBuffer;
	};


	class SolServiceProxy : public ISolServiceProxy {
	private:
		ObjectID       m_id;
		ISolService*   m_service;

		Contract       m_clientContract;
		Contract*      m_serviceContract;

	protected:

		ContractBuilder& getContractBuilder() {
			return m_clientContract.getBuilder();
		}

	public:

		SolServiceProxy(Contract* serviceContract)
		{
			m_serviceContract = serviceContract;
		}

		SolServiceProxy() {

		}

		ObjectID ID() {
			return m_id;
		}

		void ID(ObjectID id) {

			m_id = id;
		}


		Contract& getClientContract() {
			return m_clientContract;
		}

		void setService(ISolService* service) {
			m_service = service;
		}

		ObjectID preCache(std::string requestName, std::vector<SolAny*> args, std::vector<unsigned int> order) {
			return m_serviceContract->cacheArgs(requestName, args, order);
		}



		SolServiceResponse submitRequest(std::string requestName, ObjectID argBufferID, SolAny* ret) {
			Request request;

			request.callName = requestName;
			request.client = m_id;
			request.isAsync = false;
			request.argBufferID = argBufferID;
			request.retBuffer = ret;

			return m_service->submitRequest(request);
		}

		void submitRequestAsync(std::string requestName, ObjectID argBufferID, std::function<void(SolServiceResponse&)> callback) {

		}

		SolServiceResponse submitRequest(std::string requestName, ObjectID argBufferID) {
			
			Request request;

			request.callName = requestName;
			request.client = m_id;
			request.isAsync = false;
			request.argBufferID = argBufferID;
			request.retBuffer = nullptr;

			return m_service->submitRequest(request);
		}

		void finalize() {

			m_service->reloadClient(m_id);

		}
	};


	template<typename T_REPO, typename T_CLIENT_INFO>
	class Service : public ISolService {
	private:
		IBus&         m_parentBus;
		BusRepo<T_REPO>		m_repo;

		std::string			m_name;
		IEngine*            m_engine;

		Contract            m_contract;

		struct ClientState {
			
			SolServiceProxy                 	proxy;

			T_CLIENT_INFO						clientInfo;

			std::list<Request>					pendingRequests;

			ISolInterface*						solInterface;
		};

		Pool<ClientState, std::string>	 m_clients;
		

		PooledWrapper<ClientState>& getFreeClientState() {

			auto& stateWrapper = m_clients.getFree(0);

			stateWrapper.getVal().proxy.ID(stateWrapper.ID);

			return stateWrapper;
		}

		ObjectID preCache(ObjectID clientID, std::string requestName, std::vector<SolAny*> args, std::vector<unsigned int> order) {

			return m_contract.cacheArgs(requestName, args, order);
		}

	public:

		Service(IBus& bus) : m_parentBus(bus)
		{
			m_repo   = BusRepo<T_REPO>();
			m_contract = Contract();

		
		}

		~Service() {
		}


		BusRepo<T_REPO>& getRepo() {
			return m_repo;
		}

		ContractBuilder& getContractBuilder() {
		
			return m_contract.getBuilder();
		}

		PooledWrapper<ClientState>& getClientState(ObjectID id) {
			
			return m_clients.getObject(id);
		}

		IBus& getServiceBus() {
			return m_parentBus;
		}

		Contract& getServiceContract() {

			return m_contract;
		}

		void splitExecution(std::string executor) {

			getServiceBus().getServiceScheduler().addEntrypoint(this, executor);
		}

		ISolFunction* callClient(ObjectID clientId, std::string functionName) {
 			
			ClientState&  info = m_clients.getObject(clientId).getVal();
			ISolFunction* func = info.proxy.getClientContract().getFunction(functionName);

			return func;
		}

		void reloadClient(ObjectID clientID) {

			auto& clientState = getClientState(clientID).getVal();
			
			SolAnyImpl<T_CLIENT_INFO*> client_info(&clientState.clientInfo);
			SolAnyImpl<Contract*>      client_attribs(&clientState.proxy.getClientContract());

			m_contract.invokeCall("client_reload", { &client_info, &client_attribs});

		}

		bool acceptAndVerifyClientRuntimeContract(ObjectID client) {

			return true;
		}

		SolServiceResponse submitRequest(Request& request) {

			SolServiceResponse response;

			response.isValid = true;

			ClientState& clientState = m_clients.getObject(request.client).getVal();

			SolAnyImpl<ClientState*> arg_client(&clientState);

			m_contract.cacheMoreArgs(request.callName, { &arg_client }, { 0 }, request.argBufferID);
			m_contract.invokeCachedCall(request.retBuffer, request.callName, request.argBufferID);
			
			return response;
		}

		SolServiceResponse submitRequestAsync(Request& request) {

			ClientState& clientState = m_clients.getObject(request.client).getVal();
			clientState.pendingRequests.push_back(request);

			return SolServiceResponse();
		}

		std::string getName() {
			return m_name;
		}

		void setName(std::string name) {

			m_name = name;

			m_parentBus.attachService(this);
		}

		void disconnectClient(ISolServiceProxy* client) {

			m_clients.free(client->ID());

		}

		ISolServiceProxy* connectClient(ISolInterface* solInterface) {

			PooledWrapper<ClientState>& stateWrapper = getFreeClientState();
			
			ClientState& state = stateWrapper.getVal();

			state.proxy		   = SolServiceProxy(&m_contract);

			state.proxy.setService(this);
			state.proxy.ID(stateWrapper.ID);

			return &state.proxy;
		}
	};
}


