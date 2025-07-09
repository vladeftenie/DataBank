#include "../include/utils.hpp"
#include "../include/parse.hpp"
#include "../include/types.hpp"

ClientInfo unpack(const string &s) {
    size_t pos1 = s.find('|');
    size_t pos2 = s.find('|', pos1 + 1);
    size_t pos3 = s.find('|', pos2 + 1);
    size_t pos4 = s.find('|', pos3 + 1);
    size_t pos5 = s.find('|', pos4 + 1);
    size_t pos6 = s.find('|', pos5 + 1);
    size_t pos7 = s.find('|', pos6 + 1);

    ClientInfo c;
    c.zmq_identity = s.substr(0, pos1); // TODO
    c.role = string_to_role(s.substr(pos1 + 1, pos2 - pos1 - 1));
    c.id = stoi(s.substr(pos2 + 1, pos3 - pos2 - 1));
    c.cod = s.substr(pos3 + 1, pos4 - pos3 - 1);
    c.status = string_to_status(s.substr(pos4 + 1, pos5 - pos4 - 1));
    c.payload = s.substr(pos5 + 1, pos6 - pos5 - 1);
    c.action = string_to_clientAction(s.substr(pos6 + 1, pos7 - pos6 - 1));
    c.value = stod(s.substr(pos7 + 1));
    return c;
}

string pack(const ClientInfo &c) {
    return c.zmq_identity + "|" + role_to_string(c.role) + "|" + to_string(c.id) + "|" + c.cod + "|" + status_to_string(c.status) + "|" + c.payload + "|" + clientAction_to_string(c.action) + "|" + to_string(c.value);
}

void print(const ClientInfo &c) {
    cout << "  ZMQ    : " << c.zmq_identity << "\n";
    cout << "  Role   : " << role_to_string(c.role) << "\n";
    cout << "  Id     : " << c.id << "\n";
    cout << "  Cod    : " << c.cod << "\n";
    cout << "  Status : " << status_to_string(c.status) << "\n";
    cout << "  Payload: " << c.payload << "\n";
    cout << "  ClientAction : " << clientAction_to_string(c.action) << "\n";
    cout << "  Value  : " << c.value << "\n";
}
