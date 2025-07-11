#pragma once
#include <string>

using namespace std;

enum Role {
    CLIENT,
    MASTER,
    ADMIN
};

enum Status {
    REQ,
    PENDING,
    PROCESSED,
    ERROR
};

enum ClientAction {
    CONSUMA,
    ASK,
    ADD_DATE,
    ADD_BANI,
    CHANGE,
    REMOVE,
    LIST
};

struct ClientInfo {
    string zmq_identity = "";
    Role role;
    int id = 0;
    string cod = "";
    Status status;
    string payload = "";
    ClientAction action;
    double value = 0.0;
};
