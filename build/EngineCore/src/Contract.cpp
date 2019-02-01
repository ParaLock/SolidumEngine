
#include "../include/Contract.h"

ContractBuilder& Contract::getBuilder() {

    m_builder = ContractBuilder
    (
        [this](size_t size) {
            
            return nullptr;
            //@TODO: Use engine allocator here!
            //return malloc(size);//this->m_engine->getAllocator()->getMemory(size);
        },
        [this](std::vector<FuncElement>& functions) {

            this->addFunctions(functions);

        },
        [this](std::vector<std::pair<std::string, SolAny*>> attrib) {

            this->addAttribs(attrib);

        }
    );

    return m_builder;
}

std::vector<std::string> Contract::getContractKeys() {
		
	std::vector<std::string> keys = m_functions.names;
	keys.insert( keys.end(), m_attribs.names.begin(), m_attribs.names.end() );

	return keys;
}

ISolFunction* Contract::getFunction(unsigned int id) {

    return m_functions.functionList[id].function;
}

ISolFunction* Contract::getFunction(std::string name) {

    unsigned int id = elementIDByName.at(name);
    return m_functions.functionList[id].function;
}

SolAny* Contract::getMember(unsigned int id) {

    return m_attribs.attribList[id];
}

SolAny* Contract::getMember(std::string name) {

    unsigned int id = elementIDByName.at(name);
    return m_attribs.attribList[id];
}

unsigned int Contract::getCallID(std::string callName) {

    return elementIDByName.at(callName);
}

void Contract::addFunctions(std::vector<FuncElement> funcs) {

    for (int i = 0; i < funcs.size(); i++) {

        Callable call;
        auto& funcInfo = funcs.at(i);

        call.function = funcs.at(i).func;

        elementIDByName.insert({ funcs.at(i).name, m_functions.functionList.size() });

        m_functions.names.push_back(funcs.at(i).name);
        m_functions.functionList.push_back(call);

    }

}

void Contract::addAttribs(std::vector<std::pair<std::string, SolAny*>> attribs) {

    for (int i = 0; i < attribs.size(); i++) {

        elementIDByName.insert({ attribs.at(i).first, m_attribs.attribList.size() });

        m_attribs.names.push_back(attribs.at(i).first);
        m_attribs.attribList.push_back(attribs.at(i).second);

    }
}

void Contract::appendFunction(std::string name, ISolFunction* func) {

    Callable call;
    call.function = func;

    elementIDByName.insert({ name, m_functions.functionList.size() });

    m_functions.functionList.push_back(call);

}

void Contract::invokeCachedCall(SolAny* ret, std::string callName, ObjectID buffID) {
    
    invokeCachedCall(ret, elementIDByName.at(callName), buffID);
}


void Contract::invokeCachedCall(SolAny* out, unsigned int callID, ObjectID buffID) {

    auto& call = m_functions.functionList.at(callID);
    auto& buff = call.callBuffers.getObject(buffID);

    std::vector<SolAny*>& args = buff.getVal().bufferPtrs;

    if (call.function->hasRet()) {
        call.function->invoke(out, args);
    }
    else {
        call.function->invoke(args);
    }

    m_functions.functionList.at(callID).callBuffers.free(buff.ID);
}

void Contract::invokeCall(std::string callName, std::vector<SolAny*> args, SolAny* ret) {

    invokeCall(getCallID(callName), args, ret);
}

void Contract::invokeCall(unsigned int callID, std::vector<SolAny*> args, SolAny* ret) {

    auto& call = m_functions.functionList.at(callID);

    if (call.function->hasRet()) {

        call.function->invoke(ret, args);
        if (ret != nullptr)
            ret->copyTo(ret);
    }
    else {

        call.function->invoke(args);
    }

}

ObjectID Contract::cacheArgs(std::string callName, std::vector<SolAny*> args,std::vector<unsigned int> order) {

    ObjectID fakeID;
    fakeID.isActive = false;
    return cacheArgs(elementIDByName.at(callName), args, order, fakeID);
}

ObjectID Contract::cacheMoreArgs(std::string callName, std::vector<SolAny*> args, std::vector<unsigned int> order, ObjectID callBuffID) {

    return cacheArgs(elementIDByName.at(callName), args, order, callBuffID);
}

ObjectID Contract::cacheArgs(unsigned int callID, std::vector<SolAny*> args, std::vector<unsigned int> order, ObjectID callBuffID) {

    auto& call		  = m_functions.functionList.at(callID);
    auto& buffWrapper = m_functions.functionList.at(callID).callBuffers.getObjectIfValid(callBuffID);

    auto& buff		  = buffWrapper.getVal();

    if (!buff.isInitialized) {

        call.function->createCallBuffer(buff.bufferPtrs, &buff.retBuffer);
        buff.isInitialized = true;
    }

    for (int i = 0; i < order.size(); i++) {

        args[i]->copyTo(buff.bufferPtrs[order.at(i)]);
    }

    return buffWrapper.ID;
}


