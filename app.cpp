#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// Callback to collect response data from curl into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* s) {
    size_t totalSize = size * nmemb;
    s->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Function to send prompt to Groq API and get the response string
string sendChatMessage(const string& prompt, const string& apiKey, const string& apiEndpoint, const string& model) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Failed to initialize CURL." << endl;
        return "";
    }

    json payload = {
        {"model", model},
        {"messages", {
            {{"role", "user"}, {"content", prompt}}
        }}
    };

    string readBuffer;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, apiEndpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    string jsonPayload = payload.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "CURL request failed: " << curl_easy_strerror(res) << endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return "";
    }

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        cerr << "HTTP request failed with code " << http_code << endl;
        cerr << "Response body: " << readBuffer << endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return "";
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    auto jsonResponse = json::parse(readBuffer, nullptr, false);
    if (jsonResponse.is_discarded()) {
        cerr << "Failed to parse JSON response." << endl;
        return "";
    }
    
    if (jsonResponse.contains("choices") &&
        jsonResponse["choices"].is_array() &&
        !jsonResponse["choices"].empty()) {
        
        auto& firstChoice = jsonResponse["choices"][0];
        if (firstChoice.contains("message") &&
            firstChoice["message"].contains("content") &&
            firstChoice["message"]["content"].is_string()) {

            return firstChoice["message"]["content"];
        } else {
            cerr << "JSON response does not contain expected 'message.content' string." << endl;
            return "";
        }
    } else {
        cerr << "JSON response does not contain 'choices' array or it is empty." << endl;
        return "";
    }
}

int main() {
    string apiKey = "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";
    string apiEndpoint = "https://api.groq.com/openai/v1/chat/completions";
    string model = "gemma2-9b-it"; // Use a chat-optimized model

    cout << "Chatbot started. Type your message, or 'exit' to quit." << endl;

    while (true) {
        cout << "\nYou: ";
        string userInput;
        getline(cin, userInput);

        // Exit condition
        if (userInput == "exit" || userInput == "quit") {
            cout << "Exiting chat. Goodbye!" << endl;
            break;
        }

        string response = sendChatMessage(userInput, apiKey, apiEndpoint, model);

        if (!response.empty()) {
            cout << "Bot: " << response << endl;
        } else {
            cout << "Bot: (No response or error occurred)" << endl;
        }
    }

    return 0;
}
// this is the app file
