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
        cerr << "Error in string_to_role!\n";
        return;
    }
    return r;
}

string status_to_string(Status s) {
    switch (s) {
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
    if (s == "pending") {
        status = PENDING;
    } else if (s == "processed") {
        status = PROCESSED;
    } else if (s == "error") {
        status = ERROR;
    } else {
        cerr << "Error in string_to_status!\n";
        return;
    }
    return status;
}
