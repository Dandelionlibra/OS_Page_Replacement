#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <set>
#include <algorithm>
using namespace std;

// Constants for methods
const int FIFO = 1;
const int LRU = 2;
const int LFU_FIFO = 3;
const int MFU_FIFO = 4;
const int LFU_LRU = 5;
const int ALL = 6;

struct page_record {
    // int method;
    // int page;
    bool in_page_frame = false;
    int count = 0;
    int timestamp = 0;

};

struct output_data{
    string current_page = "";
    string pageFame_data = "";
    bool page_faults = false; // default
};

struct Compare {
    // LRU_Algorithm
    bool operator()(const pair<int, int>& a, const pair<int, int>& b) {
        return a.second > b.second;
    }
    // LFU_and_FIFO_Algorithm set<pair<int, pair<int, int>>> //! <page, <count, timestamp>>
    bool operator()(const pair<int, pair<int, int>>& a, const pair<int, pair<int, int>>& b) const {
        // according to count and timestamp to sort
        // if (a.second.first != b.second.first)
        //     return a.second.first < b.second.first;
        return a.second.second > b.second.second;
    }
};

bool readInputFile(string &fileName, int &method, int &pageFrames, vector<int> &pageReferences, bool &readflied) {
    if (fileName == "exit" || fileName == "Exit" || fileName == "q") {
        cout << "\033[1;31mExiting Program.\033[0m" << endl;
        return false; // end
    }
    else if(fileName.size() >= 4){
        string ext = fileName.substr(fileName.size() - 4);
        // cout << "File Name: " << fileName << endl;
        if (ext != ".txt")
            fileName = fileName + ".txt";
        // cout << "File Name: " << fileName << endl;
    }
    else
        fileName = fileName + ".txt";

    ifstream inputFile(fileName);
    if (!inputFile){
        cerr << "\033[1;31mError: Unable to open input file.\033[0m" << endl;
        readflied = true;
        return true; // not end
    }
    

    inputFile >> method >> pageFrames;
    string page;
    inputFile >> page;
    // cout << "Page: " << page << endl;
    for(int i = 0 ; i < page.size() ; i++){
        int tmp = stoi(page.substr(i, 1));
        pageReferences.push_back(tmp);
    }
    
    /*cout << "\033[1;32mInput file read successfully.\033[0m" << endl;
    cout << "Method: " << method << "  Page Frames: " << pageFrames << endl;
    cout << "Page References: ";
    for (const int& page : pageReferences) {
        cout << page << " ";
    }
    cout << endl;*/

    inputFile.close();
    return true;
}

void FIFO_Algorithm(int page_Frames, const vector<int>& page_References, ofstream& outputFile) {
    outputFile << "--------------FIFO-----------------------\n";
    
    queue<int> pageQueue;
    unordered_map<int, bool> Page_Records; // page -> isExist
    int page_Faults = 0, page_Replaces = 0;

    int ref = 0;
    for (int page : page_References) {
        // cout << "\033[1;35mPage: " << page << "\033[0m" << endl;
        output_data output_line;
        output_line.current_page = to_string(page);

        if (!Page_Records[page]) {
            page_Faults++;
            output_line.page_faults = true; // page fault
            if (pageQueue.size() >= page_Frames) { // Page frame is full
                Page_Records.erase(pageQueue.front());
                pageQueue.pop(); // Remove the first page
                page_Replaces++;
            }
            pageQueue.push(page);
            Page_Records[page] = true;
        }

        // Output current page frame state
        queue<int> tempQueue = pageQueue;
        while (!tempQueue.empty()) {
            int d = tempQueue.front();
            tempQueue.pop();
            output_line.pageFame_data += to_string(d);
        }

        // write to output file
        outputFile << output_line.current_page << "\t";
        for (int i = output_line.pageFame_data.size()-1 ; i >= 0 ; i--)
            outputFile << output_line.pageFame_data[i];
        if (output_line.page_faults == true)
            outputFile << "\tF" << "\n";
        else
            outputFile << "\n";
    }
    outputFile << "Page Fault = " << page_Faults << "  Page Replaces = " << page_Replaces << "  Page Frames = " << page_Frames << "\n";
}

int find_oldest(unordered_map<int, int>& pagePosition, const int &currnt_time) {
    int time_distance = -1;
    int oldest_Page = 0;
    for (auto it = pagePosition.begin(); it != pagePosition.end(); it++) {
        int time = currnt_time - it->second;
        if (time > time_distance) {
            time_distance = time;
            oldest_Page = it->first;
        }
    }
    // cout << "\033[1;33mOldest Page: " << oldest_Page << "\033[0m" << endl;
    return oldest_Page;
}
// Least Recently Used Page Replacement Algorithm(LRU)
void LRU_Algorithm(int pageFrames, const vector<int>& pageReferences, ofstream& outputFile) {
    outputFile << "--------------LRU-----------------------\n";
    unordered_map<int, int> pagePosition; // page -> (position, timestamp)
    int pageFaults = 0, pageReplaces = 0;

    int currnet_time = 0;
    for (int page : pageReferences) {
        output_data output_line;
        output_line.current_page = to_string(page);
        const auto it = pagePosition.find(page);
        if (it == pagePosition.end()) { // Page not found
            pageFaults++;
            output_line.page_faults = true; // page fault
            if (pagePosition.size() >= pageFrames) { // Page frame is full
                int oldest_Page = find_oldest(pagePosition, currnet_time); // Find the least recently used page
                pagePosition.erase(oldest_Page);
                pageReplaces++;
            }
            // added new page, set it's timestamp
            pagePosition[page] = currnet_time;
        }
        else // Page found, update it's timestamp
            pagePosition[page] = currnet_time;
        
        // Output current page frame state
        vector<pair<int,int>> output_sortedPages;
        for (const auto& item : pagePosition) {
            // cout << "current_time: " << currnet_time << "  Page: " << item.first << "  Time: " << item.second << endl;
            // auto pos = lower_bound(output_sortedPages.begin(), output_sortedPages.end(), make_pair(item.first, item.second),([](const pair<int,int>& a, const pair<int,int>& b) { return a.second > b.second; }));
            output_sortedPages.push_back(make_pair(item.first, item.second));
        }
        sort(output_sortedPages.begin(), output_sortedPages.end(), [](const pair<int,int>& a, const pair<int,int>& b) { return a.second > b.second; });


        // cout << "Sorted Pages: ";
        for (const auto& item : output_sortedPages) {
            // cout << item.first << " ";
            output_line.pageFame_data += to_string(item.first);
        }
        // cout << endl;

        outputFile << output_line.current_page << "\t" << output_line.pageFame_data << (output_line.page_faults ? "\tF" : "") << "\n";
        currnet_time++;
    }

    outputFile << "Page Fault = " << pageFaults << "  Page Replaces = " << pageReplaces << "  Page Frames = " << pageFrames << "\n";
}

int find_least_ot_most_frequent(unordered_map<int, page_record> pageFrames_data, int method) {
    int count_min = 9999999;
    int count_max = -1;

    int timestamp = -1;
    int least_frequent = 0;
    
    // cout << "\033[1;33mLeast Frequent Page: " << least_frequent << "\033[0m" << endl;
    for (const auto& item : pageFrames_data) {
        if (item.second.in_page_frame == false)
            continue;
        if ((item.second.count < count_min && method == LFU_FIFO) || (item.second.count > count_max && method == MFU_FIFO)) {
            count_min = item.second.count;
            count_max = item.second.count;
            timestamp = item.second.timestamp;
            least_frequent = item.first;
        }
        // if count is same, then check timestamp, find the oldest one
        else if ((method == LFU_FIFO && item.second.count == count_min && item.second.timestamp < timestamp) || (method == MFU_FIFO && item.second.count == count_max && item.second.timestamp < timestamp)) {
            timestamp = item.second.timestamp;
            least_frequent = item.first;
        }
    }
    // cout << "\033[1;33mLeast Frequent Page: " << least_frequent << "\033[0m" << endl;
    return least_frequent;
}
// Least Frequently Used (LFU) + First In First Out (FIFO)
void LFU_and_FIFO_Algorithm(int pageFrames, const vector<int>& pageReferences, ofstream& outputFile) {
    outputFile << "--------------Least Frequently Used Page Replacement-----------------------\n";
    unordered_map<int, page_record> pageRecords; // page -> (in_page_frame_flag, count, timestamp)
    set<pair<int, pair<int, int>>, Compare> pageFrames_data; // <page, <count, timestamp>>
    int pageFaults = 0, pageReplaces = 0;
    int currnet_time = 0;

    for (int page:pageReferences){
        output_data output_line;
        output_line.current_page = to_string(page);
        const auto it = pageRecords.find(page);

        if (it == pageRecords.end() || it->second.in_page_frame == false){
            pageFaults++;
            output_line.page_faults = true; // page fault
            if (pageFrames_data.size() >= pageFrames){
                // page frame is full
                int oldest_Page = find_least_ot_most_frequent(pageRecords, LFU_FIFO);
                for(auto tmp = pageFrames_data.begin(); tmp != pageFrames_data.end(); tmp++){
                    if (tmp->first == oldest_Page){
                        // cout << "\033[1;35mOldest Page: " << oldest_Page << "  count: " << tmp->second.first << "  timestamp: " << tmp->second.second << "\033[0m" << endl;
                        pageFrames_data.erase(tmp);
                        break;
                    }
                }
                pageRecords[oldest_Page].in_page_frame = false;
                pageReplaces++;
            }
            
            pageRecords[page].count++;
            pageRecords[page].in_page_frame = true;
            pageRecords[page].timestamp = currnet_time;
            pageFrames_data.insert(make_pair(page, make_pair(pageRecords[page].count, currnet_time)));
        }
        else{
            // page found, update it's count and timestamp
            // cout << "page:" << page << " count:" << pageRecords[page].count << " timestamp:" << pageRecords[page].timestamp << "\n";
            // pageFrames_data.erase(make_pair(page, make_pair(pageRecords[page].count, pageRecords[page].timestamp)));
            pageRecords[page].count++;
            pageRecords[page].timestamp = currnet_time;
            // pageFrames_data.insert(make_pair(page, make_pair(pageRecords[page].count, pageRecords[page].timestamp)));
        }

        // cout << "Current Page: " << page << "  Current Time:" << currnet_time << " \n";
        int i = 1;

        for(auto it : pageFrames_data){
            // cout << i <<  ":" << it.first << " count:" << it.second.first << " timestamp:" << it.second.second << "\n";
            output_line.pageFame_data += to_string(it.first);
        }

        // cout << endl;

        outputFile << output_line.current_page << "\t" << output_line.pageFame_data << (output_line.page_faults ? "\tF" : "") << "\n";
        currnet_time++;
    }

    outputFile << "Page Fault = " << pageFaults << "  Page Replaces = " << pageReplaces << "  Page Frames = " << pageFrames << "\n";
}

// Most Frequently Used (MFU) + First In First Out (FIFO)
void MFU_and_FIFO_Algorithm(int pageFrames, const vector<int>& pageReferences, ofstream& outputFile) {
    outputFile << "--------------Most Frequently Used Page Replacement -----------------------\n";
    unordered_map<int, page_record> pageRecords; // page -> (in_page_frame_flag, count, timestamp)
    set<pair<int, pair<int, int>>, Compare> pageFrames_data; // <page, <count, timestamp>>
    int pageFaults = 0, pageReplaces = 0;
    int currnet_time = 0;

    for (int page:pageReferences){
        output_data output_line;
        output_line.current_page = to_string(page);
        const auto it = pageRecords.find(page);

        if (it == pageRecords.end() || it->second.in_page_frame == false){
            pageFaults++;
            output_line.page_faults = true; // page fault
            if (pageFrames_data.size() >= pageFrames){
                // page frame is full
                int oldest_Page = find_least_ot_most_frequent(pageRecords, MFU_FIFO);
                for(auto tmp = pageFrames_data.begin(); tmp != pageFrames_data.end(); tmp++){
                    if (tmp->first == oldest_Page){
                        cout << "\033[1;35mOldest Page: " << oldest_Page << "  count: " << tmp->second.first << "  timestamp: " << tmp->second.second << "\033[0m" << endl;
                        pageFrames_data.erase(tmp);
                        break;
                    }
                }
                pageRecords[oldest_Page].in_page_frame = false;
                pageReplaces++;
            }
            
            pageRecords[page].count++;
            pageRecords[page].in_page_frame = true;
            pageRecords[page].timestamp = currnet_time;
            pageFrames_data.insert(make_pair(page, make_pair(pageRecords[page].count, currnet_time)));
        }
        else{
            // page found, update it's count and timestamp
            cout << "page:" << page << " count:" << pageRecords[page].count << " timestamp:" << pageRecords[page].timestamp << "\n";
            // pageFrames_data.erase(make_pair(page, make_pair(pageRecords[page].count, pageRecords[page].timestamp)));
            pageRecords[page].count++;
            pageRecords[page].timestamp = currnet_time;
            // pageFrames_data.insert(make_pair(page, make_pair(pageRecords[page].count, pageRecords[page].timestamp)));
        }

        cout << "Current Page: " << page << "  Current Time:" << currnet_time << " \n";
        int i = 1;

        for(auto it : pageFrames_data){
            cout << i <<  ":" << it.first << " count:" << it.second.first << " timestamp:" << it.second.second << "\n";
            output_line.pageFame_data += to_string(it.first);
        }

        cout << endl;
    

        outputFile << output_line.current_page << "\t" << output_line.pageFame_data << (output_line.page_faults ? "\tF" : "") << "\n";
        currnet_time++;
    }
    outputFile << "Page Fault = " << pageFaults << "  Page Replaces = " << pageReplaces << "  Page Frames = " << pageFrames << "\n";
}

void LFU_and_LRU_Algorithm(int pageFrames, const vector<int>& pageReferences, ofstream& outputFile) {
    outputFile << "--------------Least Frequently Used LRU Page Replacement-----------------------\n";
    unordered_map<int, int> pagePosition; // page -> (position, timestamp)
    int pageFaults = 0, pageReplaces = 0;

    int currnet_time = 0;

    outputFile << "Page Fault = " << pageFaults << "  Page Replaces = " << pageReplaces << "  Page Frames = " << pageFrames << "\n";
}

int main() {
    string inputFileName;
    while(1){
        cout << "Please enter File Name (eg. input1, input1.txt): ";
        cin >> inputFileName;

        int method, pageFrames;
        bool readflied = false;
        vector<int> pageReferences;

        // Read input file
        if (!readInputFile(inputFileName, method, pageFrames, pageReferences, readflied)) // if readflied is false, then exit
            break;
        if (readflied)
            continue;

        // Open output file
        string outputFileName = "out_" + inputFileName;
        ofstream outputFile(outputFileName);
        if (!outputFile)
            cerr << "Error: Unable to create output file." << endl;

        // Execute selected method
        switch (method) {
            case FIFO:
                FIFO_Algorithm(pageFrames, pageReferences, outputFile);
                break;
            case LRU:
                LRU_Algorithm(pageFrames, pageReferences, outputFile);
                break;
            case LFU_FIFO:
                LFU_and_FIFO_Algorithm(pageFrames, pageReferences, outputFile);
                break;
            case MFU_FIFO:
                MFU_and_FIFO_Algorithm(pageFrames, pageReferences, outputFile);
                break;
            case LFU_LRU:
                LFU_and_LRU_Algorithm(pageFrames, pageReferences, outputFile);
                break;
            case ALL:
                FIFO_Algorithm(pageFrames, pageReferences, outputFile);
                outputFile << "\n";
                LRU_Algorithm(pageFrames, pageReferences, outputFile);
                outputFile << "\n";
                LFU_and_FIFO_Algorithm(pageFrames, pageReferences, outputFile);
                outputFile << "\n";
                MFU_and_FIFO_Algorithm(pageFrames, pageReferences, outputFile);
                outputFile << "\n";
                LFU_and_LRU_Algorithm(pageFrames, pageReferences, outputFile);
                break;
            default:
                cerr << "Error: Invalid method." << endl;
                break;
        }

        outputFile.close();
        cout << "\033[1;32mOutput written to " << outputFileName << "\033[0m" << endl;
        break;
    }

    return 0;
}
