#include "Book.h"
#include "Common.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <algorithm> // Needed for std::min
#include <stdexcept> // For exceptions in parsing

namespace Book {

    // --- Internal Data ---
    std::vector<std::vector<Move>> bookVariations;
    bool loaded = false;
    std::mt19937 rng(std::random_device{}());

    // --- Helper Functions for Algebraic Notation ---
    std::string moveToAlgebraic(const Move& move) {
        if (move.fromCol < 0 || move.fromCol >= BOARD_COLS || move.fromRow < 0 || move.fromRow >= BOARD_ROWS ||
            move.toCol < 0 || move.toCol >= BOARD_COLS || move.toRow < 0 || move.toRow >= BOARD_ROWS) return "xxxx";
        std::string s;
        s += (char)('a' + move.fromCol); s += std::to_string(move.fromRow + 1);
        s += (char)('a' + move.toCol); s += std::to_string(move.toRow + 1);
        return s;
    }
    Move algebraicToMove(const std::string& algNote) {
        if (algNote.length() != 4) throw std::invalid_argument("Algebraic notation len != 4");
        int c1 = algNote[0] - 'a', r1 = algNote[1] - '1', c2 = algNote[2] - 'a', r2 = algNote[3] - '1';
        if (c1<0||c1>=BOARD_COLS||r1<0||r1>=BOARD_ROWS||c2<0||c2>=BOARD_COLS||r2<0||r2>=BOARD_ROWS)
            throw std::invalid_argument("Invalid coords: " + algNote);
        return {r1, c1, r2, c2};
    }

    // --- Main Book Functions ---

    // Load function remains largely the same
    bool load(const std::string& filename) {
        bookVariations.clear(); // Clear existing data before loading
        loaded = false;
        std::ifstream inFile(filename);
        if (!inFile.is_open()) return false;

        std::string line;
        int lineNumber = 0;
        int variationsLoaded = 0;
        while (std::getline(inFile, line)) {
            lineNumber++;
            if (line.empty() || line[0] == '#') continue;
            std::vector<Move> currentVariation;
            std::stringstream ss(line);
            std::string moveStr;
            bool parseError = false;
            while (ss >> moveStr) {
                try { currentVariation.push_back(algebraicToMove(moveStr)); }
                catch (const std::invalid_argument& e) {
                    std::cerr << "Book Load Error (Line " << lineNumber << "): " << e.what() << std::endl;
                    parseError = true; break;
                }
            }
            if (!parseError && !currentVariation.empty()) {
                bookVariations.push_back(currentVariation);
                variationsLoaded++;
            }
        }
        inFile.close();
        if (variationsLoaded > 0) {
            loaded = true;
            // Avoid printing during load called by saveVariation? Add a flag?
            // For now, let's assume it's okay.
            // std::cout << "Opening book '" << filename << "' loaded (" << variationsLoaded << " lines)." << std::endl;
        }
        return loaded;
    }

    // Find book move remains the same
    Move findBookMove(const std::vector<Move>& moveSequence) {
        if (!loaded || bookVariations.empty()) return {-1, -1, -1, -1};
        std::vector<Move> candidateMoves;
        size_t plyCount = moveSequence.size();
        for (const auto& variation : bookVariations) {
            if (variation.size() > plyCount) {
                bool match = true;
                for (size_t i = 0; i < plyCount; ++i) { if (!(variation[i] == moveSequence[i])) { match = false; break; } }
                if (match) candidateMoves.push_back(variation[plyCount]);
            }
        }
        if (!candidateMoves.empty()) {
            std::uniform_int_distribution<size_t> dist(0, candidateMoves.size() - 1);
            return candidateMoves[dist(rng)];
        }
        return {-1, -1, -1, -1};
    }

    bool isLoaded() {
        return loaded;
    }

    // <<< NEW saveVariation IMPLEMENTATION >>>
    SaveResult saveVariation(const std::vector<Move>& newSequence, const std::string& filename) {
        if (newSequence.empty()) {
            std::cerr << "Book Editor Warning: Cannot save an empty move sequence." << std::endl;
            return SaveResult::ERROR_EMPTY;
        }

        // Ensure the book is loaded into memory for checking
        if (!loaded) {
            load(filename); // Attempt to load if not already loaded
        }

        int replaceIndex = -1; // Index of the line to replace (-1 means append)
        bool alreadyExists = false;

        // Check against existing variations
        for (size_t i = 0; i < bookVariations.size(); ++i) {
            const auto& existingVariation = bookVariations[i];
            if (existingVariation.empty()) continue;

            size_t minLen = std::min(newSequence.size(), existingVariation.size());
            bool prefixMatch = true;
            for (size_t j = 0; j < minLen; ++j) {
                if (!(newSequence[j] == existingVariation[j])) {
                    prefixMatch = false;
                    break;
                }
            }

            if (prefixMatch) {
                // Case 1: New sequence is identical or shorter than existing
                if (newSequence.size() <= existingVariation.size()) {
                    alreadyExists = true;
                    break; // No need to check further
                }
                // Case 2: New sequence extends this existing line
                else { // newSequence.size() > existingVariation.size()
                    // Found a line to replace. Take the first one found.
                    replaceIndex = static_cast<int>(i);
                    break; // Assume we only replace one line
                }
            }
        }

        // If identical or shorter line exists, do nothing
        if (alreadyExists) {
            return SaveResult::ALREADY_EXISTS;
        }

        // --- Perform File Operation ---

        // If replacing, rewrite the whole file
        if (replaceIndex != -1) {
            std::vector<std::string> allLines;
            std::string line;
            std::ifstream inFile(filename);
            // Read existing lines (even if inFile fails, vector remains empty)
            if (inFile.is_open()) {
                 while (std::getline(inFile, line)) { allLines.push_back(line); }
                 inFile.close();
            }

            // Check if replaceIndex is valid (should be if file was read)
             if (replaceIndex < 0 || replaceIndex >= allLines.size()) {
                 std::cerr << "Book Editor Error: Invalid index (" << replaceIndex << ") for replacement." << std::endl;
                 // Fallback to appending? Or return error? Let's return error.
                 return SaveResult::ERROR_FILE; // Indicate an internal logic/state error
             }

            // Format the new line
            std::ostringstream ossNew;
            for (size_t i = 0; i < newSequence.size(); ++i) {
                ossNew << moveToAlgebraic(newSequence[i]) << (i < newSequence.size() - 1 ? " " : "");
            }
            allLines[replaceIndex] = ossNew.str(); // Replace the line content

            // Rewrite the file
            std::ofstream outFile(filename, std::ios::trunc); // Truncate mode
            if (!outFile.is_open()) {
                std::cerr << "Book Editor Error: Could not open file '" << filename << "' for rewriting." << std::endl;
                return SaveResult::ERROR_FILE;
            }
            for (const auto& l : allLines) { outFile << l << "\n"; }
            outFile.close();

            if (outFile.fail()) {
                std::cerr << "Book Editor Error: Failed writing updated book to '" << filename << "'." << std::endl;
                load(filename); // Try to reload original state
                return SaveResult::ERROR_FILE;
            }

            load(filename); // Reload in-memory variations
            return SaveResult::UPDATED;

        }
        // Otherwise, append the new line
        else {
            std::ofstream outFile(filename, std::ios::app); // Append mode
            if (!outFile.is_open()) {
                std::cerr << "Book Editor Error: Could not open file '" << filename << "' for appending." << std::endl;
                return SaveResult::ERROR_FILE;
            }
            for (size_t i = 0; i < newSequence.size(); ++i) {
                outFile << moveToAlgebraic(newSequence[i]) << (i < newSequence.size() - 1 ? " " : "");
            }
            outFile << "\n"; // Add newline
            outFile.close();

             if (outFile.fail()) {
                 std::cerr << "Book Editor Error: Failed appending to file '" << filename << "'." << std::endl;
                 load(filename); // Reload state before failed append
                 return SaveResult::ERROR_FILE;
             }

            load(filename); // Reload in-memory variations
            return SaveResult::APPENDED;
        }
    }

    // Removed checkVariationExists as its logic is now inside saveVariation

} // namespace Book


