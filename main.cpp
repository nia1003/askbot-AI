#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "Storage.h"
#include "Callbacks.h"
#include <memory>

Storage storage; // Instantiate the Storage object

using namespace std;

void askQuestion(string api_key = "OPENAI_API_KEY", string endpoint = "https://api.openai.com/v1/chat/completions")
{
    std::string question;
    getline(cin, question);
    if (question.empty())
    {
        cout << "請輸入問題" << endl;
        askQuestion(api_key, endpoint);
    }
    else if (question == "print")
    {
        cout << endl << "以下是歷史紀錄: " << endl;
        storage.printRecords();
    }
    else if (question == "search")
    {
        cout << endl << "請輸入關鍵字:";
        std::string query;
        getline(cin, query);
        storage.findRelevance(query);
    }
    else if (question == "exit")
    {
        cout << "再見!" << endl;
        exit(0);
    }
    else
    {
        std::string post_data = R"({
            "model": "gpt-4o",
            "messages": [
                {"role": "system", "content": "You are a helpful assistant. Answer the question in 200 words or less."},
                {"role": "user", "content": ")" +
                                question + R"("}
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

            try {
                res = curl_easy_perform(curl);
                if (res != CURLE_OK)
                {
                    throw std::runtime_error(curl_easy_strerror(res));
                }
                else
                {
                    auto json_response = nlohmann::json::parse(response_data);
                    std::string answer = json_response["choices"][0]["message"]["content"];
                    std::cout << endl << "【 AI response 🤖 】\n"
                              << answer << std::endl;

                    // add to storage
                    auto newRecord = std::make_shared<Record>(question, answer); // Use smart pointer to manage Record
                    storage.addRecord(*newRecord);
                }
            } catch (const std::exception& e) {
                std::cerr << "cURL Error: " << e.what() << std::endl;
            }

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
    }
    cout << endl;
}

std::string api_key;
std::string endpoint;

int main()
{
    while (true)
    {   cout << "-----------------------------------------------------\n" << endl;
        cout << "Welcome to askbot AI! I am your agent 🤖." << endl;
        cout << endl;
        cout << "Instructions:" << endl;

        cout << "I. You can type in questions directly." << endl;
        cout << "II. Type 'print' to see all history records." << endl;
        cout << "III. Type 'search' to search for relevant history records." << endl;
        cout << "IV. Type 'exit' to exit the program." << endl;
        cout << "-----------------------------------------------------" << endl;
        askQuestion();
    }

    return 0;
}
