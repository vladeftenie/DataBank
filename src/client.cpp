#include "../include/utils.hpp"
#include "../include/parse.hpp"

int main(int argc, char *argv[]) {
    if(argc < 3) {
        cerr << "Not enough args!\n";
        return -1;
    }

    ClientInfo c;
    c.role = CLIENT;
    c.id = stoi(argv[1]);
    c.cod = argv[2];
    c.status = REQ;
    c.action = string_to_clientAction(argv[3]);
    if(c.action == ASK) {
        c.payload = argv[4];
    } else {
        c.value = stod(argv[4]);
    }

    context_t context(NUM_CONTEXT);
    socket_t socket(context, socket_type::dealer);
    socket.set(sockopt::routing_id, "client-" + to_string(c.id));
    socket.connect("tcp://localhost:5555");

    string packed = pack(c);
    message_t request(packed.begin(), packed.end());
    socket.send(request, send_flags::none);

    message_t reply;
    if(!socket.recv(reply)) {
        cerr << "EROARE la request recv\n";
        return;
    }

    string reply_str((char *)reply.data(), reply.size());
    c = unpack(reply_str);
    print(c);

    socket.close();
    context.shutdown();
    context.close();
}
