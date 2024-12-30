#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <vector>
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <memory>

using namespace std;

struct Record {
    std::string question;
    std::string answer;
    Record(std::string q, std::string a) : question(q), answer(a) {}
};

class Storage {
public:
    Storage();
    void addRecord(const Record& newRecord);
    void printRecords();
    vector<shared_ptr<Record>> getRecords();
    vector<shared_ptr<Record>> findRelevance(std::string query);  
    ~Storage() = default;

private:
    std::vector<shared_ptr<Record>> records;

    template<typename T>
    vector<shared_ptr<Record>> findRelevanceTemplate(T query);
};

#endif // STORAGE_H
