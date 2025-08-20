#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <set>
#include <numeric>
#include <cmath>
#include <random>

// Include windows.h for console coloring, with guards for non-Windows platforms
#ifdef _WIN32
#include <windows.h>
#endif

// --- 1. Console Color Management ---

const int COLOR_GREEN = 10;
const int COLOR_CYAN = 11;
const int COLOR_RED = 12;
const int COLOR_YELLOW = 14;
const int COLOR_WHITE = 15;
const int COLOR_DEFAULT = 7;

void setColor(int color) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
#else
    if (color == COLOR_RED) std::cout << "\033[1;31m";
    else if (color == COLOR_YELLOW) std::cout << "\033[1;33m";
    else if (color == COLOR_GREEN) std::cout << "\033[1;32m";
    else if (color == COLOR_CYAN) std::cout << "\033[1;36m";
    else if (color == COLOR_WHITE) std::cout << "\033[1;37m";
    else std::cout << "\033[0m";
#endif
}

void resetColor() {
#ifdef _WIN32
    setColor(COLOR_DEFAULT);
#else
    std::cout << "\033[0m";
#endif
}

// --- 2. Semantic Analysis Helper Functions ---

double jaccardSimilarity(const std::set<std::string>& set1, const std::set<std::string>& set2) {
    std::vector<std::string> intersection;
    std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(),
                          std::back_inserter(intersection));
    std::vector<std::string> set_union;
    std::set_union(set1.begin(), set1.end(), set2.begin(), set2.end(),
                   std::back_inserter(set_union));
    if (set_union.empty()) return 0.0;
    return static_cast<double>(intersection.size()) / set_union.size();
}

std::string formatJoinedSequence(const std::vector<std::string>& words, int start_index, int end_index) {
    std::string result;
    for (int i = start_index; i <= end_index && i < words.size(); ++i) {
        result += words[i] + " ";
    }
    return result;
}

// --- 3. The Transition Analyzer Function ---

void analyzeForStableTransitions(const std::string& text) {
    setColor(COLOR_WHITE);
    std::cout << "\n--- Transition Analyzer Activated ---\n";
    resetColor();

    std::vector<std::string> all_words;
    std::stringstream text_stream(text);
    std::string word; // Moved declaration here
    while (text_stream >> word) {
        word.erase(std::remove_if(word.begin(), word.end(), ispunct), word.end());
        std::transform(word.begin(), word.end(), word.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if (!word.empty()) {
            all_words.push_back(word);
        }
    }

    const int SEQUENCE_LENGTH = 5;
    if (all_words.size() < SEQUENCE_LENGTH * 3) { return; }

    std::vector<std::set<std::string>> sequences;
    for (size_t i = 0; i <= all_words.size() - SEQUENCE_LENGTH; ++i) {
        sequences.emplace_back(all_words.begin() + i, all_words.begin() + i + SEQUENCE_LENGTH);
    }

    std::vector<double> similarities, rates_of_change;
    for (size_t i = 1; i < sequences.size(); ++i) {
        similarities.push_back(jaccardSimilarity(sequences[i-1], sequences[i]));
    }
    for (size_t i = 1; i < similarities.size(); ++i) {
        rates_of_change.push_back(similarities[i] - similarities[i-1]);
    }

    const double VALLEY_THRESHOLD = 0.1, STABILITY_THRESHOLD = 0.1;
    int transitions_found = 0;

    for (size_t i = 1; i < rates_of_change.size(); ++i) {
        if (std::abs(rates_of_change[i-1]) > VALLEY_THRESHOLD) {
            for (size_t j = i; j < rates_of_change.size(); ++j) {
                if (std::abs(rates_of_change[j]) < STABILITY_THRESHOLD) {
                    transitions_found++;
                    int start_idx = i, end_idx = j + SEQUENCE_LENGTH;
                    setColor(COLOR_YELLOW);
                    std::cout << "Transition #" << transitions_found << " Detected (from word " << start_idx + 1 << " to " << end_idx + 1 << "):\n";
                    std::cout << "  - Joined Sequence: \"";
                    setColor(COLOR_WHITE);
                    std::cout << formatJoinedSequence(all_words, start_idx, end_idx);
                    resetColor();
                    std::cout << "\"\n\n";
                    i = j;
                    break;
                }
            }
        }
    }

    if (transitions_found == 0) {
        setColor(COLOR_GREEN);
        std::cout << "No significant semantic transitions were found in this text configuration." << std::endl;
        resetColor();
    }
}

// --- 4. Main Program Entry Point ---

int main() {
    // 1. Get user input for the text.
    setColor(COLOR_CYAN);
    std::string original_filename = "test.txt";


    // 2. Perform the text perturbation.
    std::ifstream file_in(original_filename);
    std::stringstream buffer;
    buffer << file_in.rdbuf();
    std::string original_text = buffer.str();
    file_in.close();

    // 2. Get user input for markers.
    setColor(COLOR_CYAN);
    std::cout << "Enter the START marker: ";
    resetColor();
    std::string start_marker;
    std::cin >> start_marker;

    setColor(COLOR_CYAN);
    std::cout << "Enter the END marker: ";
    resetColor();
    std::string end_marker;
    std::cin >> end_marker;

    // 3. Find all possible substrings between any start and end marker.
    std::vector<std::string> possible_sequences;
    size_t start_pos = original_text.find(start_marker);
    while (start_pos != std::string::npos) {
        size_t end_pos = original_text.find(end_marker, start_pos);
        while (end_pos != std::string::npos) {
            possible_sequences.push_back(original_text.substr(start_pos, end_pos - start_pos + end_marker.length()));
            end_pos = original_text.find(end_marker, end_pos + 1);
        }
        start_pos = original_text.find(start_marker, start_pos + 1);
    }

    if (possible_sequences.empty()) {
        setColor(COLOR_RED);
        std::cerr << "Error: No valid substring found between the given markers." << std::endl;
        resetColor();
        return 1;
    }

    // 4. Randomly select one substring to move.
    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<> distrib(0, possible_sequences.size() - 1);
    std::string mobile_sequence = possible_sequences[distrib(g)];

    setColor(COLOR_YELLOW);
    std::cout << "\nRandomly selected sequence to move: \"" << mobile_sequence << "\"\n";
    resetColor();

    // 5. Create base text and perform the perturbation.
    std::string base_text = original_text;
    size_t mobile_pos = base_text.find(mobile_sequence);
    if (mobile_pos != std::string::npos) {
        base_text.erase(mobile_pos, mobile_sequence.length());
    }

    std::vector<std::string> base_words;
    std::stringstream base_stream(base_text);

    // FIX: Declare 'word' outside the loop so it's in the correct scope.
    std::string word;
    while(base_stream >> word) { base_words.push_back(word); }

    std::uniform_int_distribution<> insert_distrib(0, base_words.size());
    int insertion_word_index = insert_distrib(g);

    std::string perturbed_text;
    for (int i = 0; i < base_words.size(); ++i) {
        if (i == insertion_word_index) {
            perturbed_text += mobile_sequence + " ";
        }
        perturbed_text += base_words[i] + " ";
    }
    if (insertion_word_index >= base_words.size()) {
        perturbed_text += mobile_sequence;
    }

    // 6. Analyze the resulting text.
    setColor(COLOR_CYAN);
    std::cout << "\n--- Analyzing perturbed text with moved sequence ---" << std::endl;
    analyzeForStableTransitions(perturbed_text);

    return 0;
}
