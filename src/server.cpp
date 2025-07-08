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

void parse(ClientInfo &c, unique_ptr<Connection> &con) {
    try {
        unique_ptr<PreparedStatement> stmt(
            con->prepareStatement("SELECT status, pret FROM files WHERE filename = ?")
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
}

void listener(int id) {

}

void parser(int id) {

}

void sender(int id) {

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
    return 0;
}
