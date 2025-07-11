#include "../include/utils.hpp"
#include "../include/parse.hpp"
// ./admin 1 change 2080... nume date_consumate date_available bani
int main(int argc, char *argv[]) {
    if(argc < 3) {
        cerr << "Not enough args!\n";
        return -1;
    }

    ClientInfo c;
    c.role = ADMIN;
    c.id = stoi(argv[1]);
    c.action = string_to_clientAction(argv[2]);
    if (c.action == LIST) {
        cout << "DA\n";
    } else {
        c.cod = argv[3];
        if (c.action == ASK) {
            c.payload = argv[4];
        } else if (c.action == CHANGE) {
           c.payload = string(argv[4]) + " " + string(argv[5]) + " " + string(argv[6]) + " " + string(argv[7]) + " " + string(argv[8]);
        } else if (c.action == ADD_DATE || c.action == ADD_BANI) {
           c.value = stod(argv[4]);
        }
    }

    c.status = REQ;

    context_t context(NUM_CONTEXT);
    socket_t socket(context, socket_type::dealer);
    socket.set(sockopt::routing_id, "admin-" + to_string(c.id));
    socket.connect("tcp://localhost:5555");

    string packed = pack(c);
    cout << packed << "\n";
    message_t request(packed.begin(), packed.end());
    socket.send(request, send_flags::none);

    message_t reply;
    if(!socket.recv(reply)) {
        cerr << "EROARE la request recv\n";
        return -1;
    }

    string reply_str((char *)reply.data(), reply.size());
    c = unpack(reply_str);
    print(c);

    socket.close();
    context.shutdown();
    context.close();
}
