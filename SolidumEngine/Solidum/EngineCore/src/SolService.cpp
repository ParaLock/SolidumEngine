#include "../include/SolService.h"


SolServiceResponse SolService::SolServiceProxy::submitRequest(bool isBuilderCall, std::string requestName)
{
	Request request;

	request.callName = requestName;
	request.client   = m_id;
	request.isAsync  = false;
	request.isBuilderCall = isBuilderCall;

	return m_service->submitRequest(request);
}

void SolService::SolServiceProxy::submitRequestAsync(bool isBuilderCall, std::string requestName, std::function<void(SolServiceResponse&)> callback)
{
	
}
