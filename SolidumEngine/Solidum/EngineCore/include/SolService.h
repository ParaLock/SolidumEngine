#pragma once

#include <deque>

#include "../../../SolidumAPI/include/EngineAPI.h"
#include "../../../SolidumAPI/include/ServiceAPI.h"
#include "../../../SolidumAPI/include/ResourceAPI.h"

namespace SolService {

	struct Request {

		bool           isAsync;
		bool           isBuilderCall;

		ClientID       client;
		std::string    callName;

	};

	class ISolService {
	public:
		virtual SolServiceResponse   submitRequest(Request& request) = 0;

		virtual std::string          getName() = 0;
		virtual void		     setName(std::string name) = 0;

		virtual ISolServiceProxy*    connectClient() = 0;
		virtual void		     disconnectClient(ISolServiceProxy*) = 0;
	};

	class SolServiceProxy : public ISolServiceProxy {
	private:
		ClientID       m_id;

		ISolService*   m_service;
	public:

		ClientID ID() {
			return m_id;
		}

		void ID(ClientID id) {
			m_id = id;
		}

		void setService(ISolService* service) {
			m_service = service;
		}

		SolServiceResponse submitRequest(bool isBuilderCall, std::string requestName);
		void			   submitRequestAsync(bool isBuilderCall, std::string requestName, std::function<void(SolServiceResponse&)> callback);

		bool acceptAndVerifyRuntimeContract(ISolContract* contract) {
			return false;
		}

		void endBuilder() {

			SolServiceResponse response = submitRequest(true, "BUILDER_FINALIZE");
	
			response.call->invokeBound();

		}
	};

	template<typename T_CLIENT_INFO, typename T_CLIENT_BUILDER>
	class Service : public ISolService {
	private:

		ClientID    m_lastID;

		std::string m_name;

		struct CallInfo {

			ISolFunction* callFunctor;
		};

		std::map<std::string, CallInfo> m_calls;

		struct ClientState {

			ClientState() {}

			ClientState(const ClientState& other) {

				proxy = other.proxy;
				clientInfo = other.clientInfo;
				currentClientBuilder = other.currentClientBuilder;

				pendingRequests = other.pendingRequests;
			}

			SolServiceProxy    proxy;

			T_CLIENT_INFO      clientInfo;
			T_CLIENT_BUILDER   currentClientBuilder;

			std::list<Request> pendingRequests;
		};

		std::deque<ClientState> m_clients;

		ISolFunction* getBuilderCall(Request& request) {

			ClientState& client = m_clients.at(request.client);

			ISolContract* contract = &client.currentClientBuilder.DynamicContract;

			auto call = contract->getFunctions().at(request.callName);

			if (request.callName == "BUILDER_FINALIZE") {
				
				call->bindNext(SolAnyImpl<T_CLIENT_INFO*>(&client.clientInfo));
				call->bindNext(SolAnyImpl<T_CLIENT_BUILDER*>(&client.currentClientBuilder));
			}

			return call;
		}
	public:

		Service() {
			m_lastID = -1;
		}

		~Service() {
		}

		void registerContract(ISolContract* contract) {

			auto calls = contract->getFunctions();

			for (auto itr = calls.begin(); itr != calls.end(); itr++) {
				
				CallInfo info;
				info.callFunctor = itr->second;

				m_calls.insert({itr->first, info});
			}
		}

		SolServiceResponse submitRequest(Request& request) {

			SolServiceResponse response;

			response.isValid = true;

			ClientState& clientState = m_clients.at(request.client);

			if (request.isBuilderCall) {

				response.call = getBuilderCall(request);
			}
			else {

				ISolFunction* call = m_calls.at(request.callName).callFunctor;

				call->bindNext(SolAnyImpl<T_CLIENT_INFO*>(&clientState.clientInfo));

				response.call = call;
				
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

			//@IMPLEMENT, lol

		}

		ISolServiceProxy* connectClient() {

			m_lastID++;

			m_clients.emplace_back(ClientState());

			ClientState& state = m_clients.back();

			state.currentClientBuilder.init();
			state.proxy.ID(m_lastID);
			state.proxy.setService(this);

			state.currentClientBuilder.DynamicContract.getFunctions().insert({"BUILDER_FINALIZE", m_calls.at("BUILDER_FINALIZE").callFunctor });

			return &state.proxy;

		}
	};
}


