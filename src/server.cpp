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

void send_pack(ClientInfo &c) { //TODO: to use
    string packed = pack(c);
    message_t request(packed.begin(), packed.end());
    socket.send(request, send_flags::none);
}

void parse_change(const string &s, string &nume, int &date_c, int &date_a, double &bani) {
    size_t pos1 = s.find(' ');
    size_t pos2 = s.find(' ', pos1 + 1);
    size_t pos3 = s.find(' ', pos2 + 1);
    size_t pos4 = s.find(' ', pos3 + 1);

    nume = s.substr(0, pos2);
    date_c = stoi(s.substr(pos2 + 1, pos3 - pos2 - 1));
    date_a = stoi(s.substr(pos3 + 1, pos4 - pos3 - 1));
    cout << date_c << " " << date_a << "\n";
    bani = stod(s.substr(pos4 + 1));
}

void parse(ClientInfo &c, unique_ptr<Connection> &con) {
    if (c.action == CONSUMA) {
        if (c.role == ADMIN) {
            c.payload = "Comanda inexistenta";
            c.status = ERROR;
            return;
        }
        double cerere = c.value;
        
        unique_ptr<PreparedStatement> stmt(
            con->prepareStatement("SELECT date_available, bani FROM clients WHERE imsi = ? OR msisdn = ?")
        );

        stmt->setString(1, c.cod);
        stmt->setString(2, c.cod);

        unique_ptr<ResultSet> res(stmt->executeQuery());

        if (!res->next()) {
            c.payload = "Client inexistent";
            c.status = ERROR;
            return;
        }

        double disponibile = res->getDouble("date_available");
        double bani = res->getDouble("bani");

        if (disponibile >= cerere) {
            unique_ptr<PreparedStatement> update(
                con->prepareStatement("UPDATE clients SET date_available = ?, date_consumate = date_consumate + ? WHERE imsi = ? OR msisdn = ?")
            );
            update->setInt(1, disponibile - cerere);
            update->setInt(2, cerere);
            update->setString(3, c.cod);
            update->setString(4, c.cod);
            update->executeUpdate();

            c.payload = "Ai consumat " + to_string(cerere) + " MB. Mai ai " + to_string(disponibile - cerere) + " MB disponibili luna asta.";
            c.status = PROCESSED;
            c.value = 0.0;
        } else {
            double lipsa = cerere - disponibile;
            double cost = lipsa * 0.05;     // 100MB = 5$

            if (bani >= cost) {
                unique_ptr<PreparedStatement> update(
                    con->prepareStatement("UPDATE clients SET date_available = 0, bani = bani - ?, date_consumate = date_consumate + ? WHERE imsi = ? OR msisdn = ?")
                );
                update->setDouble(1, cost);
                update->setInt(2, cerere);
                update->setString(3, c.cod);
                update->setString(4, c.cod);
                update->executeUpdate();

                c.payload = "Ai consumat " + to_string(cerere) + " MB.\nAi platit " + to_string(cost) + " bani pentru " + to_string(lipsa) + " MB in plus.";
                c.status = PROCESSED;
                c.value = 0.0;
            } else {
                c.payload = "Nu ai suficiente date sau bani. \nTrebuie sa platesti " + to_string(cost) + " bani pentru a consuma " + to_string(cerere) + " MB.";
                c.status = PENDING;
            }
        }
    } else if (c.action == ASK) {
        string col = c.payload;
        unordered_set<string> allowed = {"bani", "date_consumate", "date_available"};
        if (!allowed.count(col)) {
            c.payload = "Coloană invalidă";
            c.status = ERROR;
        }
    
        string query = "SELECT " + col + " FROM clients WHERE imsi = ? OR msisdn = ?";
        unique_ptr<PreparedStatement> stmt(
            con->prepareStatement(query)
        );
        stmt->setString(1, c.cod);
        stmt->setString(2, c.cod);
        
        unique_ptr<ResultSet> res(stmt->executeQuery());
    
        if (!res->next()) {
            c.payload = "Client inexistent";
            c.status = ERROR;
            return;
        }

        double val = res->getDouble(col);
        cout << col << " = " << val << "\n";
        c.value = val;

        c.status = PROCESSED;
    } else if (c.action == ADD_BANI) {
        if (c.role == ADMIN) {
            unique_ptr<PreparedStatement> update(
                con->prepareStatement("UPDATE clients SET bani = bani + ? WHERE imsi = ? OR msisdn = ?")
            );
            update->setDouble(1, c.value);
            update->setString(2, c.cod);
            update->setString(3, c.cod);
            int affectedRows = update->executeUpdate();

            if (!affectedRows) {
                c.status = ERROR;
                c.payload = "Client inexistent";
                return;
            }
            c.status = PROCESSED;
        } else {
            unique_ptr<PreparedStatement> stmt(
                con->prepareStatement("INSERT INTO pending_requests (imsi, msisdn, request_type, payload) VALUES (?, ?, ?, ?)")
            );
        
            if (c.cod.size() < 13) {
                stmt->setString(1, c.cod);
                stmt->setNull(2, DataType::VARCHAR);
            } else {
                stmt->setNull(1, DataType::VARCHAR);
                stmt->setString(2, c.cod);
            }
            stmt->setString(3, "ADD_BANI");
            stmt->setString(4, to_string(c.value));
            stmt->executeUpdate();
    
            c.status = PENDING;
        }

    } else if (c.action == ADD_DATE) {
        if (c.role == ADMIN) {
            unique_ptr<PreparedStatement> update(
                con->prepareStatement("UPDATE clients SET date_available = date_available + ? WHERE imsi = ? OR msisdn = ?")
            );
            update->setInt(1, c.value);
            update->setString(2, c.cod);
            update->setString(3, c.cod);
            int affectedRows = update->executeUpdate();

            if (!affectedRows) {
                c.status = ERROR;
                c.payload = "Client inexistent";
                return;
            }
            c.status = PROCESSED;
        } else {
            unique_ptr<PreparedStatement> stmt(
                con->prepareStatement("INSERT INTO pending_requests (imsi, msisdn, request_type, payload) VALUES (?, ?, ?, ?)")
            );
        
            if (c.cod.size() < 13) {
                stmt->setString(1, c.cod);
                stmt->setNull(2, DataType::VARCHAR);
            } else {
                stmt->setNull(1, DataType::VARCHAR);
                stmt->setString(2, c.cod);
            }
            stmt->setString(3, "ADD_DATE");
            stmt->setString(4, to_string(c.value));
            stmt->executeUpdate();
    
            c.status = PENDING;
        }
    } else if (c.action == CHANGE) {
        if (c.role == ADMIN) {
            unique_ptr<PreparedStatement> update(
                con->prepareStatement("UPDATE clients SET nume_client = ?, date_consumate = ?, date_available = ?, bani = ? WHERE imsi = ? OR msisdn = ?")
            );

            string nume;
            int date_c, date_a;
            double bani;
            parse_change(c.payload, nume, date_c, date_a, bani);
            update->setString(1, nume);
            update->setInt(2, date_c);
            update->setInt(3, date_a);
            update->setDouble(4, bani);
            update->setString(5, c.cod);
            update->setString(6, c.cod);
            int affectedRows = update->executeUpdate();

            if (!affectedRows) {
                c.status = ERROR;
                c.payload = "Client inexistent";
                return;
            }
            c.status = PROCESSED;
        }
    } else if (c.action == REMOVE) {
        unique_ptr<PreparedStatement> update(
            con->prepareStatement("DELETE FROM clients WHERE imsi = ? OR msisdn = ?")
        );

        update->setString(1, c.cod);
        update->setString(2, c.cod);
        int affectedRows = update->executeUpdate();

        if (!affectedRows) {
            c.status = ERROR;
            c.payload = "Client inexistent";
            return;
        }
        c.status = PROCESSED;
    } else if (c.action == LIST) {
        unique_ptr<PreparedStatement> stmt(
            con->prepareStatement("SELECT * FROM clients")
        );

        unique_ptr<ResultSet> res(stmt->executeQuery());

        if (!res->next()) {
            c.payload = "Client inexistent";
            c.status = ERROR;
            return;
        }
        
        string payload = "";
        do {
            payload += "$ ";
            // payload += res->getString("id") + " | ";
            payload += res->getString("imsi") + " $ ";
            payload += res->getString("msisdn") + " $ ";
            payload += res->getString("nume_client") + " $ ";
            payload += to_string(res->getDouble("date_consumate")) + " $ ";
            payload += to_string(res->getDouble("date_available")) + " $ ";
            payload += to_string(res->getDouble("bani")) + " $\n";
        } while (res->next());
        c.payload = payload;
        c.status = PROCESSED;
        print(c);
    } else {
        cout << "Nu\n";
    }
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
