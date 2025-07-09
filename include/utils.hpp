#pragma once

#include <zmq.hpp>
#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <csignal>
#include <atomic>

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

#include "types.hpp"

#define LIMIT 15000
#define NUM_CONTEXT 2

#define FILER_THREADS 2
#define LISTENER_THREADS 1
#define PARSER_THREADS 4
#define SENDER_THREADS 2

// using namespace std;
using namespace zmq;
using namespace sql;

ClientInfo unpack(const string &s);
string pack(const ClientInfo &c);

void print(const ClientInfo &c);
