#pragma once

#include "../../Containers/include/ObjectPool.h"

#include "../../../SolidumAPI/include/EngineAPI.h"
#include "../../../SolidumAPI/include/ServiceAPI.h"
#include "../../../SolidumAPI/include/ResourceAPI.h"

using namespace ObjectPool;

namespace SolService {

	struct Request {

		Request() {
			ret = nullptr;
		}

		bool           isAsync;
		bool           isBuilderCall;

		ObjectID       client;
		std::string    callName;

		std::vector<void*>          args;
		void*                       ret;

		ElementBuffer               argBuffer;
	};

	class ISolService {
	public:
		virtual SolServiceResponse   submitRequest(Request& request) = 0;

		virtual std::string          getName() = 0;
		virtual void				 setName(std::string name) = 0;

		virtual ISolServiceProxy*    connectClient() = 0;
		virtual void				 disconnectClient(ISolServiceProxy*) = 0;

		virtual bool				 acceptAndVerifyClientRuntimeContract(ObjectID client, ISolContract* contract) = 0;
	};

	class SolServiceProxy : public ISolServiceProxy {
	private:
		ObjectID       m_id;

		ISolService*   m_service;
	public:

		ObjectID ID() {
			return m_id;
		}

		void ID(ObjectID id) {
			m_id = id;
		}

		void setService(ISolService* service) {
			m_service = service;
		}

		SolServiceResponse submitRequest(bool isBuilderCall, std::string requestName, std::vector<void*> args, void* ret);
		void			   submitRequestAsync(bool isBuilderCall, std::string requestName, std::vector<void*> args, void* ret, std::function<void(SolServiceResponse&)> callback);

	    bool acceptAndVerifyRuntimeContract(ISolContract* contract) {

			return m_service->acceptAndVerifyClientRuntimeContract(m_id, contract);
		}

		void endBuilder() {

			SolServiceResponse response = submitRequest(true, "BUILDER_FINALIZE", {}, nullptr);

		}
	};

	template<typename T_CLIENT_INFO, typename T_CLIENT_BUILDER, typename T_DYNAMIC_CONTRACT>
	class Service : public ISolService {
	private:

		T_DYNAMIC_CONTRACT  DYNAMIC_CONTRACT;

		std::string			m_name;
		IEngine*            m_engine;

		struct CallInfo {

			ISolFunction* callFunctor;
		};

		struct ClientState {

			ClientState() {
			
				runtimeContract = nullptr;
			
			}

			ClientState(const ClientState& other) {

				proxy				 = other.proxy;
				clientInfo			 = other.clientInfo;
				currentClientBuilder = other.currentClientBuilder;

				pendingRequests		 = other.pendingRequests;
			}

			ISolContract*      runtimeContract;

			SolServiceProxy    proxy;

			T_CLIENT_INFO      clientInfo;
			T_CLIENT_BUILDER   currentClientBuilder;

			std::list<Request> pendingRequests;
		};

		Pool<ClientState, std::string>	 m_clients;
		
		PooledWrapper<ClientState>& getFreeClientState() {

			auto& stateWrapper = m_clients.getFree(0);

			stateWrapper.getVal().proxy.ID(stateWrapper.ID);

			return stateWrapper;
		}

		void callBuilder(Request& request) {

			PooledWrapper<ClientState>& client = m_clients.getObject(request.client);

			auto& contract = client.getVal().currentClientBuilder.DynamicContract;

			auto call = contract.getFunctions().at(request.callName);

			if (request.callName == "BUILDER_FINALIZE") {
				
				T_CLIENT_BUILDER* builder = &client.getVal().currentClientBuilder;
				T_CLIENT_INFO*    clientInfo = &client.getVal().clientInfo;

				request.args.push_back(clientInfo);
				request.args.push_back(builder);
			}

			call->invoke(request.args);
		}


	public:

		Service(IEngine* engine)
		{
			m_engine = engine;
		}

		~Service() {
		}

		T_DYNAMIC_CONTRACT& getDynamicContract() {
			return DYNAMIC_CONTRACT;
		}

		PooledWrapper<ClientState>& getClientState(ObjectID id) {
			return m_clients.getObject(id);
		}

		ISolFunction* callClient(ObjectID id, std::string functionName) {
			return m_clients.getObject(id).getVal().runtimeContract->getFunctions().at(functionName);
		}

		bool acceptAndVerifyClientRuntimeContract(ObjectID client, ISolContract* contract) {

			m_clients.getObject(client).getVal().runtimeContract = contract;

			return true;
		}

		SolServiceResponse submitRequest(Request& request) {

			SolServiceResponse response;

			response.isValid = true;

			PooledWrapper<ClientState>& clientState = m_clients.getObject(request.client);

			if (request.isBuilderCall) {

				callBuilder(request);
			}
			else {

				if (request.isAsync) {

					ISolFunction* call = DYNAMIC_CONTRACT.getFunctions().at(request.callName);
					ElementBuffer buff = DYNAMIC_CONTRACT.getElementBuffer(request.callName);

					call->argCopy(buff.mem, request.args);

					request.argBuffer = buff;

					clientState.getVal().pendingRequests.push_back(request);
				}
				else {

					ISolFunction* call = DYNAMIC_CONTRACT.getFunctions().at(request.callName);

					request.args.insert(request.args.begin(), &clientState.getVal().clientInfo);

					if (request.ret == nullptr) {
						call->invoke(request.args);
					}
					else {
						call->invoke(request.ret, request.args);
					}
				}
			}

			return response;
		}

		std::string getName() {
			return m_name;
		}

		void setName(std::string name) {
			m_name = name;
		}

		void disconnectClient(ISolServiceProxy* client) {

			m_clients.free(client->ID());

		}

		ISolServiceProxy* connectClient() {

			PooledWrapper<ClientState>& stateWrapper = getFreeClientState();
			ClientState& state = stateWrapper.getVal();

			state.currentClientBuilder.init();
			state.proxy.setService(this);

			state.currentClientBuilder.DynamicContract.getFunctions().insert({"BUILDER_FINALIZE", DYNAMIC_CONTRACT.getFunctions().at("BUILDER_FINALIZE") });

			return &state.proxy;

		}

		IEngine* getEngine() {
			return m_engine;
		}
	};
}


