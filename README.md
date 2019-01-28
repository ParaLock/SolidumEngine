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

# Contracts

A critically important component of the SolidumEngine framework is what I call the dynamic contract. A dynamic contract is a unique data structure which allows a given object to build an outword facing representation of both its functionality as well as it's properties relative to another system. Ok so what does that even mean.. Well let me give you an example of a dynamic contract:



# Client/Service Model

The SolidumEngine framework is built around a Client/Service model. What this means is that the engine provides to the user a set of powerful services. These services provide, well, services such as resource management, job scheduling, inter-object communication and most importantly, "executive" and "feature" code paths (More on this below).


