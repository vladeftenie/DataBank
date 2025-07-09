#include "../include/utils.hpp"

context_t context (NUM_CONTEXT);
socket_t socket(context, socket_type::router);

queue<ClientInfo> req, rsp;
mutex req_mtx, rsp_mtx, socket_mtx;
condition_variable req_cv, rsp_cv;

atomic<bool> running(true);

mysql::MySQL_Driver *driver = mysql::get_mysql_driver_instance();

unique_ptr<Connection> connect_to_db() {
    unique_ptr<Connection> local_con(driver->connect("tcp://127.0.0.1:3306", "andrei.eftenie.pract", "ParolaTa!"));
    local_con->setSchema("databank");
    return local_con;
}

void signal_handler(int signal) {
    if (signal == SIGINT) {
        cout << "\n[SERVER] Received SIGINT, shutting down...\n";
        running = false;
        req_cv.notify_all();
        rsp_cv.notify_all();
    }
}

void send_pack(const ClientInfo &c) { //TODO: to use
    string packed = pack(c);
    message_t request(packed.begin(), packed.end());
    socket.send(request, send_flags::none);
}

void parse(ClientInfo &c, unique_ptr<Connection> &con) {
        switch (c.action) {
        case CONSUMA:
            
        case ASK:
            
        case ADD_MONEY:
            
        case ADD_DATE:
            
        default:
            return "none";
    }
    try {
        unique_ptr<PreparedStatement> stmt(
            con->prepareStatement("SELECT  FROM clients WHERE filename = ?")
        );
        // stmt->setString(1, p.filename);
        
        unique_ptr<ResultSet> res(stmt->executeQuery());
        
        if (res->next()) {
            string status = res->getString("status");
            double pret = res->getDouble("pret");
            
            if (status == "finished") {
                cout << "[DB] Fișierul este finalizat. Preț: " << pret << endl;
                // p.pret = pret;
                // p.status = FIN;
            } else {
                cout << "[DB] Fișierul NU este finalizat. Status: " << status << ". Preț: 0.00" << endl;
                // p.pret = 0.00;
                // p.status = PRS;
            }
        } else {
            // cout << "[DB] Fișierul nu a fost găsit în DB: " << p.filename << endl;
            // p.pret = 0.00;
            // p.status = NOT;
        }
    } catch (sql::SQLException &e) {
        // cerr << "[DB] Eroare la interogare în DB: " << e.what() << " pentru " << p.filename << endl;
    }
    cout << "DA\n\n\n\n";
}

void listener(int id) {
    socket.set(sockopt::rcvtimeo, 100);
    while(running) {
        message_t identity, request;
        if(!socket.recv(identity)) {
            if(!running) break;
            continue;
        }
        if(!socket.recv(request)) {
            if(!running) break;
            continue;
        }

        string content((char *)request.data(), request.size());
        ClientInfo c = unpack(content);
        c.zmq_identity = string((char *)identity.data(), identity.size());
        cout << "[Listener " << id << "]\n";
        print(c);

        {
            lock_guard<mutex> lock(req_mtx);
            req.push(c);
        }
        req_cv.notify_one();
    }
}

void parser(int id) {
    unique_ptr<Connection> local_con = connect_to_db();

    while(running) {
        ClientInfo c;
        {
            unique_lock<mutex> lock(req_mtx);
            req_cv.wait(lock, [] { return !req.empty() || !running; });
            if(!running && req.empty()) break;
            if(req.empty()) continue;

            c = req.front();
            req.pop();
        }

        cout << "[Worker " << id << "] Procesare " << c.cod << endl;
        parse(c, local_con);

        {
            lock_guard<mutex> lock(rsp_mtx);
            rsp.push(c);
        }
        rsp_cv.notify_one();
    }
}

void sender(int id) {
    while(running) {
        ClientInfo c;
        {
            unique_lock<mutex> lock(rsp_mtx);
            rsp_cv.wait(lock, [] { return !rsp.empty() || !running; });
            if (!running) break;
            
            c = rsp.front();
            rsp.pop();
        }

        message_t identity(c.zmq_identity.begin(), c.zmq_identity.end());
        string packed = pack(c);
        message_t response(packed.begin(), packed.end());
        
        {
            lock_guard<mutex> lock(socket_mtx);  // 🔒
            socket.send(identity, send_flags::sndmore);
            socket.send(response, send_flags::none);
        }
        
        cout << "[Sender " << id << "] Trimis răspuns pentru " << c.cod << endl;
    }
}

int main() {
    signal(SIGINT, signal_handler);
    
    socket.bind ("tcp://*:5555");
    
    vector<thread> listeners;
    for(int i = 0; i < LISTENER_THREADS; i++) {
        listeners.emplace_back(listener, i);
    }
    
    vector<thread> parsers;
    for(int i = 0; i < PARSER_THREADS; i++) {
        parsers.emplace_back(parser, i);
    }

    vector<thread> senders;
    for(int i = 0; i < SENDER_THREADS; i++) {
        senders.emplace_back(sender, i);
    }

    for(thread &l : listeners) {
        l.join();
    }

    for(thread &p : parsers) {
        p.join();
    }

    for(thread &s : senders) {
        s.join();
    }

    socket.close();
    context.shutdown();
    context.close();
    return 0;
}
