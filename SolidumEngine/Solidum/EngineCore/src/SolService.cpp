#include "../include/SolService.h"


SolServiceResponse SolService::SolServiceProxy::submitRequest(bool isBuilderCall, std::string requestName, std::vector<void*> args, void* ret)
{
	Request request;

	request.callName = requestName;
	request.client   = m_id;
	request.isAsync  = false;
	request.isBuilderCall = isBuilderCall;

	request.ret			  = ret;
	request.args		  = args;


	return m_service->submitRequest(request);
}

void SolService::SolServiceProxy::submitRequestAsync(bool isBuilderCall, std::string requestName, std::vector<void*> args, void* ret, std::function<void(SolServiceResponse&)> callback)
{
	
}
