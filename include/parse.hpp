#pragma once

#include <string>
#include "types.hpp"

string role_to_string(Role r);
Role string_to_role(const string &s);
string status_to_string(Status s);
Status string_to_status(const string &s);
string clientAction_to_string(ClientAction a);
ClientAction string_to_clientAction(const string &s);
