#pragma once
#include <string>

using namespace std;

enum Role {
    CLIENT,
    MASTER,
    ADMIN
};

enum Status {
    PENDING,
    PROCESSED,
    ERROR
};

struct ClientInfo {
    string zmq_identity;
    Role role;
    int id;
    string cod;
    Status status;
    string payload;
};
