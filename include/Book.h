#pragma once // Standard header guard

#include "Common.h" // For Move struct
#include <vector>
#include <string>
#include <random> // For random selection

// Define the namespace
namespace Book {

    // Enum to indicate the result of trying to save a variation
    // Ensure this is defined *before* its first use (which it is, in saveVariation's return type)
    enum class SaveResult {
        ERROR_EMPTY,        // Cannot save an empty sequence
        ERROR_FILE,         // Could not open/write to the file
        APPENDED,           // New variation was added successfully
        UPDATED,            // An existing shorter variation was replaced
        ALREADY_EXISTS      // Identical or shorter variation already present
    }; // <-- Semicolon is optional here for enum class, but ensure no syntax errors around it.

    // Loads the opening book from the specified file.
    bool load(const std::string& filename = "opening_book.txt");

    // Finds a book move based on the sequence of moves played so far.
    Move findBookMove(const std::vector<Move>& moveSequence);

    // Returns true if the book was loaded successfully.
    bool isLoaded();

    // Saves the variation: appends if new, updates if it extends an existing line.
    SaveResult saveVariation(const std::vector<Move>& moveSequence, const std::string& filename = "opening_book.txt");

    // Helper functions used internally and potentially by main.cpp
    std::string moveToAlgebraic(const Move& move);
    Move algebraicToMove(const std::string& algNote); // Throws std::invalid_argument

} // namespace Book


