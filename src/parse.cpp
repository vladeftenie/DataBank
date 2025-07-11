#include <iostream>

#include "../include/parse.hpp"
#include "../include/types.hpp"


string role_to_string(Role r) {
    switch (r) {
        case CLIENT:
            return "client";
        case MASTER:
            return "master";
        case ADMIN:
            return "admin";
        default:
            return "none";
    }
}

Role string_to_role(const string &s) {
    Role r;
    if (s == "client") {
        r = CLIENT;
    } else if (s == "master") {
        r = MASTER;
    } else if (s == "admin") {
        r = ADMIN;
    } else {
        throw invalid_argument("Invalid role string: " + s);
    }
    return r;
}

string status_to_string(Status s) {
    switch (s) {
        case REQ:
            return "requested";
        case PENDING:
            return "pending";
        case PROCESSED:
            return "processed";
        case ERROR:
            return "error";
        default:
            return "none";
    }
}

Status string_to_status(const string &s) {
    Status status;
    if (s == "requested") {
        status = REQ;
    } else if (s == "pending") {
        status = PENDING;
    } else if (s == "processed") {
        status = PROCESSED;
    } else if (s == "error") {
        status = ERROR;
    } else {
        throw invalid_argument("Invalid status string: " + s);
    }
    return status;
}

string clientAction_to_string(ClientAction a) {
    switch (a) {
        case CONSUMA:
            return "consuma";
        case ASK:
            return "ask";
        case ADD_BANI:
            return "add_bani";
        case ADD_DATE:
            return "add_date";
        case CHANGE:
            return "change";
        case REMOVE:
            return "remove";
        case LIST:
            return "list";
        default:
            return "none";
    }
}

ClientAction string_to_clientAction(const string &s) {
    ClientAction a;
    if (s == "consuma") {
        a = CONSUMA;
    } else if (s == "ask") {
        a = ASK;
    } else if (s == "add_bani") {
        a = ADD_BANI;
    } else if (s == "add_date") {
        a = ADD_DATE;
    } else if (s == "change") {
        a = CHANGE;
    } else if (s == "remove") {
        a = REMOVE;
    } else if (s == "list") {
        a = LIST;
    } else {
        throw invalid_argument("Invalid status string: " + s);
    }
    return a;
}
