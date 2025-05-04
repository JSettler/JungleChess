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
    // Definition of the storage for book variations (declared extern or accessed via getter)
    std::vector<std::vector<Move>> bookVariations;
    bool loaded = false;
    std::mt19937 rng(std::random_device{}()); // Random number generator

    // --- Helper Functions for Algebraic Notation ---

    // Converts internal Move struct to algebraic string (e.g., "a1b2")
    std::string moveToAlgebraic(const Move& move) {
        if (move.fromCol < 0 || move.fromCol >= BOARD_COLS ||
            move.fromRow < 0 || move.fromRow >= BOARD_ROWS ||
            move.toCol < 0 || move.toCol >= BOARD_COLS ||
            move.toRow < 0 || move.toRow >= BOARD_ROWS) {
            return "xxxx"; // Invalid move indicator
        }
        std::string s = "";
        s += (char)('a' + move.fromCol);
        s += std::to_string(move.fromRow + 1); // Row 0 is rank '1'
        s += (char)('a' + move.toCol);
        s += std::to_string(move.toRow + 1);   // Row 0 is rank '1'
        return s;
    }

    // Converts algebraic string (e.g., "a1b2") to internal Move struct
    // Throws std::invalid_argument on parsing error.
    Move algebraicToMove(const std::string& algNote) {
        if (algNote.length() != 4) {
            throw std::invalid_argument("Algebraic notation must be 4 characters long (e.g., a1b2)");
        }

        int c1 = algNote[0] - 'a';
        int r1 = algNote[1] - '1'; // Rank '1' is row 0
        int c2 = algNote[2] - 'a';
        int r2 = algNote[3] - '1'; // Rank '1' is row 0

        // Validate parsed coordinates
        if (c1 < 0 || c1 >= BOARD_COLS || r1 < 0 || r1 >= BOARD_ROWS ||
            c2 < 0 || c2 >= BOARD_COLS || r2 < 0 || r2 >= BOARD_ROWS) {
            throw std::invalid_argument("Invalid coordinates in algebraic notation: " + algNote);
        }

        return {r1, c1, r2, c2};
    }


    // --- Main Book Functions ---

    // Load function remains largely the same, using algebraicToMove
    bool load(const std::string& filename) {
        bookVariations.clear(); // Clear existing data before loading
        loaded = false;
        std::ifstream inFile(filename);
        if (!inFile.is_open()) {
            // std::cout << "Info: Opening book file '" << filename << "' not found." << std::endl;
            return false; // Silently fail if book doesn't exist
        }

        std::string line;
        int lineNumber = 0;
        int variationsLoaded = 0;
        while (std::getline(inFile, line)) {
            lineNumber++;
            if (line.empty() || line[0] == '#') continue; // Skip empty/comment lines

            std::vector<Move> currentVariation;
            std::stringstream ss(line);
            std::string moveStr;
            bool parseError = false;

            while (ss >> moveStr) {
                try {
                    Move currentMove = algebraicToMove(moveStr);
                    currentVariation.push_back(currentMove);
                } catch (const std::invalid_argument& e) {
                    // Report error but continue trying to load other lines
                    std::cerr << "Book Load Error (Line " << lineNumber << "): " << e.what() << " - Skipping move '" << moveStr << "'" << std::endl;
                    // Optionally break here if one error should invalidate the whole line:
                    // parseError = true; break;
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
            // Let's only print on initial load maybe? For now, print always.
            std::cout << "Opening book '" << filename << "' loaded (" << variationsLoaded << " lines)." << std::endl;
        } else {
            // std::cout << "Info: Opening book '" << filename << "' is empty or contained only errors." << std::endl;
        }
        return loaded;
    }

    // Find book move remains the same
    Move findBookMove(const std::vector<Move>& moveSequence) {
        if (!loaded || bookVariations.empty()) {
            return {-1, -1, -1, -1};
        }

        std::vector<Move> candidateMoves;
        size_t plyCount = moveSequence.size();

        for (const auto& variation : bookVariations) {
            if (variation.size() > plyCount) {
                bool match = true;
                for (size_t i = 0; i < plyCount; ++i) {
                    if (!(variation[i] == moveSequence[i])) { // Use overloaded == for Move
                        match = false;
                        break;
                    }
                }
                if (match) {
                    candidateMoves.push_back(variation[plyCount]);
                }
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

    // <<< NEW: Getter implementation >>>
    const std::vector<std::vector<Move>>& getVariations() {
        // Ensure book is loaded before returning? Or rely on caller checking isLoaded()?
        // Let's rely on the caller checking isLoaded() if they need valid data.
        return bookVariations;
    }


    // saveVariation implementation handles append/update logic
    SaveResult saveVariation(const std::vector<Move>& newSequence, const std::string& filename) {
        if (newSequence.empty()) {
            // std::cerr << "Book Editor Warning: Cannot save an empty move sequence." << std::endl;
            return SaveResult::ERROR_EMPTY;
        }

        // Ensure the book is loaded into memory for checking
        // load() handles clearing bookVariations first
        load(filename); // Reload fresh from file before checking/saving

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
                    // Make sure we haven't already decided to replace another line
                    // (though the break should prevent this unless logic changes)
                    if (replaceIndex == -1) {
                         replaceIndex = static_cast<int>(i);
                    }
                    // If we want to handle multiple possible replacements, logic needs adjustment.
                    // For now, replace the first one found that is shorter.
                    break;
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
            // Update the in-memory representation first
             if (replaceIndex < 0 || replaceIndex >= bookVariations.size()) {
                 std::cerr << "Book Editor Error: Invalid index (" << replaceIndex << ") for replacement (internal error)." << std::endl;
                 return SaveResult::ERROR_FILE; // Indicate an internal logic/state error
             }
            bookVariations[replaceIndex] = newSequence; // Replace in memory

            // Rewrite the file from the modified in-memory data
            std::ofstream outFile(filename, std::ios::trunc); // Truncate mode
            if (!outFile.is_open()) {
                std::cerr << "Book Editor Error: Could not open file '" << filename << "' for rewriting." << std::endl;
                load(filename); // Attempt to reload original state on error
                return SaveResult::ERROR_FILE;
            }
            // Write comments or header? Optional.
            // outFile << "# Jungle Chess Opening Book\n";
            for (const auto& variation : bookVariations) {
                 if (variation.empty()) continue; // Skip writing empty lines if any snuck in
                 for (size_t i = 0; i < variation.size(); ++i) {
                    outFile << moveToAlgebraic(variation[i]) << (i < variation.size() - 1 ? " " : "");
                 }
                 outFile << "\n";
            }
            outFile.close();

            if (outFile.fail()) {
                std::cerr << "Book Editor Error: Failed writing updated book to '" << filename << "'." << std::endl;
                load(filename); // Try to reload original state
                return SaveResult::ERROR_FILE;
            }
            // No need to call load() again, bookVariations is already updated
            loaded = true; // Ensure loaded flag is true
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
                 // Don't reload here, as the append failed. State remains as before append attempt.
                 return SaveResult::ERROR_FILE;
             }

            // Add the new variation to the in-memory list as well
            bookVariations.push_back(newSequence);
            loaded = true; // Ensure loaded flag is true
            return SaveResult::APPENDED;
        }
    }

} // namespace Book


