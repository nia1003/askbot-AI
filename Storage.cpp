#include "Storage.h"
#include "Callbacks.h"
#include <algorithm>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>

Storage::Storage() : records() {
    // Initialize records vector
}

void Storage::addRecord(const Record& newRecord) {
    try {
        records.push_back(make_shared<Record>(newRecord));
    } catch (const std::exception& e) {
        std::cerr << "Error adding record: " << e.what() << std::endl;
    }
}

void Storage::printRecords() {
    try {
        size_t max_range = std::min(static_cast<size_t>(100), records.size());
        if (records.empty()) {
            std::cout << "No records found." << std::endl;
            return;
        }
        for (size_t i = 0; i < max_range; ++i) {
            std::cout << "Question ❔: " << records[i]->question << "\n";
            std::cout << "Answer 💡: " << records[i]->answer << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error printing records: " << e.what() << std::endl;
    }
}

vector<shared_ptr<Record>> Storage::getRecords() {
    try {
        return records;
    } catch (const std::exception& e) {
        std::cerr << "Error getting records: " << e.what() << std::endl;
        return {};
    }
}

vector<shared_ptr<Record>> Storage::findRelevance(std::string query) {
    return findRelevanceTemplate(query);
}

template<typename T>
vector<shared_ptr<Record>> Storage::findRelevanceTemplate(T query) {
    const std::string api_key = "OPENAI_API_KEY"; // Replace with your actual API key
    const std::string endpoint = "https://api.openai.com/v1/chat/completions"; // Replace with your actual endpoint

    Storage relRecordStorage; // Initialize relRecordStorage using constructor
    auto records = getRecords();
    if (query.empty()){
        std::cout << "您的問題為空，請重新搜尋" << std::endl;
        std::getline(std::cin, query);
        return findRelevanceTemplate(query);
    }
    else {
        for (const auto& record : records) {
        std::string post_data = R"({
            "model": "gpt-4o",
            "messages": [
                {"role": "system", "content": "return pure text 'true' if the question is highly relevant to the query else return 'false'."},
                {"role": "user", "content": ")" + query + R"("},
                {"role": "user", "content": ")" + record->question + R"("}
            ]
        })";

        std::string response_data;
        CURL *curl;
        CURLcode res;
        curl = curl_easy_init();

        if (curl)
        {
            struct curl_slist *headers = nullptr;
            headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");

            curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

            res = curl_easy_perform(curl);
            if(res != CURLE_OK) {
                throw std::runtime_error(curl_easy_strerror(res));
            } else {
                auto json_response = nlohmann::json::parse(response_data);
                std::cout << json_response << std::endl;
                std::string rel = json_response["choices"][0]["message"]["content"];

                if (rel == "true") {
                    relRecordStorage.addRecord(*record);
                }
            }
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            }
        }
    }
    if (relRecordStorage.getRecords().empty()){
        std::cout << "找不到相關問題。" << std::endl;
        return {};
    }
    else {
        std::cout << "以下是相關問題: " << std::endl;
        relRecordStorage.printRecords();
        return relRecordStorage.getRecords();
    }
}

