#include <iostream>
#include <curl/curl.h>
#include <json/json.h>

using namespace std;

// This function is called by libcurl when there is data received
size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::string* response = reinterpret_cast<std::string*>(userdata);
    response->append(ptr, size * nmemb);
    return size * nmemb;
}
// function to perform the API request and return the JSON response
string makeRequest(string url) {
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode result = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        return response;
    }
}

// function to get address summary
void getAddressSummary(string address) {
    string url = "https://blockchain.info/address/" + address + "?format=json";
    string response = makeRequest(url);
    if (response.empty()) {
        cout << "Error: failed to fetch address data" << endl;
        exit(0);
    }
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();

    Json::Value root;
    string errors;

    bool parsingSuccessful = reader->parse(response.c_str(), response.c_str() + response.size(), &root, &errors);
    delete reader;
    if (!parsingSuccessful) {
        cout << "Error: failed to parse address data" << endl;
        exit(0);
    }
    double balance = root["final_balance"].asDouble() / 100000000;
    double sent = root["total_sent"].asDouble() / 100000000;
    double received = root["total_received"].asDouble() / 100000000;
    int n_tx = root["n_tx"].asInt();
    cout << "Address summary:" << endl;
    cout << "Address: " << address << endl;
    cout << "Balance: " << balance << " BTC" << endl;
    cout << "Sent: " << sent << " BTC" << endl;
    cout << "Received: " << received << " BTC" << endl;
    cout << "Number of transactions: " << n_tx << endl;
}

// function to get transaction summary
void getTransactionSummary(string trxid) {
    string url = "https://blockchain.info/rawtx/" + trxid;
    string response = makeRequest(url);
    if (response.empty()) {
        cout << "Error: failed to fetch transaction data" << endl;
        exit(0);
    }
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();

    Json::Value root;
    string errors;

    bool parsingSuccessful = reader->parse(response.c_str(), response.c_str() + response.size(), &root, &errors);
    delete reader;
    if (!parsingSuccessful) {
        cout << "Error: failed to parse transaction data" << endl;
        exit(0);
    }
    string status = root["block_height"].isNull() ? "Pending" : "Confirmed";
    double value = root["out"][0]["value"].asDouble() / 100000000;
    cout << "Transaction summary:" << endl;
    cout << "Transaction ID: " << trxid << endl;
    cout << "Status: " << status << endl;
    cout << "Value: " << value << " BTC" << endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " -A address | -T trxid>" << endl;
        return 0;
    }

    string option = argv[1];
    string input = argv[2];

    if (option == "-A") {
        getAddressSummary(input);
    }
    else if (option == "-T") {
        getTransactionSummary(input);
    }
    else {
        cout << "Invalid option: " << option << endl;
    }
}

