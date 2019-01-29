# SolidumEngine

The SolidumEngine is an applications development platform designed for the agile business environment.

The following is a simplified diagram of the Solidum architecture: **More detailed specification provided below diagram.**

![Spec](sol_spec.jpg)

# Architecture Overview

The SolidumEngine architecture is based on the following primary design considerations: 
1) Framework must implement a flexible and data oriented API which provides powerful features to the client whilst having the ability to mutate during runtime, based on the requirements of the API client.
2) Framework API must be EXTREMELY non intrusive.
3) Framework must harness the power of a data driven paradigm (eg. Those present in game engines) to allow for the rapid, efficient and robust developement of all manner of generic applications.
4) Framework must address some of the primary limitations present in modern App Dev frameworks such as Qt5.
5) Framework must provide a level of language interoperability.

## Client/Service Model

The SolidumEngine framework is built around a Client/Service model. What this means is that the engine provides to the user a set of powerful services. These services provide, well, services such as resource management, job scheduling, inter-object communication and most importantly, "executive" and "feature" code paths (More on this below).

## Contracts

A critically important component of the SolidumEngine framework is the dynamic contract. A dynamic contract is a unique data structure which defines the dynamic relationship between a client and a service. The meaning behind "dynamic" in this context is that, depending on what behavior and attributes the client specifies in the contract, the service may treat the client differently. For example: if I have some client of lets say the "ResourceService", and this client wants to be loadable from disk, said client may specify "isLoadable" = true in their contract. This however does not mean that all clients must be loadable from disk. Furthermore not all clients need be aware that there is a possible isLoadable attribute available to them.

### Contract Calling Convention

Further side note on contracts: All callable contract elements support full marshalling of arguments. This means that filter stages may be applied to all contract element invokations, plus, contract calls may be cached or otherwise modified at runtime in a very flexible manner.

**For an example of contract use, look at main.cpp in the root project directory**




