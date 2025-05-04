#pragma once // Standard header guard

#include "Common.h" // For Move struct
#include <vector>
#include <string>
#include <random> // For random selection
#include <stdexcept> // For algebraicToMove exception

// Define the namespace
namespace Book {

    // Enum to indicate the result of trying to save a variation
    enum class SaveResult {
        ERROR_EMPTY,        // Cannot save an empty sequence
        ERROR_FILE,         // Could not open/write to the file
        APPENDED,           // New variation was added successfully
        UPDATED,            // An existing shorter variation was replaced
        ALREADY_EXISTS      // Identical or shorter variation already present
    };

    // Loads the opening book from the specified file.
    // Parses algebraic notation (e.g., "a1b1 c7c6").
    // Returns true if loading was successful and the book is not empty.
    bool load(const std::string& filename = "opening_book.txt");

    // Finds a book move based on the sequence of moves played so far.
    // Returns a valid Move if found, otherwise returns {-1,-1,-1,-1}.
    Move findBookMove(const std::vector<Move>& moveSequence);

    // Returns true if the book was loaded successfully.
    bool isLoaded();

    // Saves the variation: appends if new, updates if it extends an existing line.
    // Returns enum indicating the result.
    SaveResult saveVariation(const std::vector<Move>& moveSequence, const std::string& filename = "opening_book.txt");

    // <<< NEW: Getter for the loaded book variations (Recommended Method) >>>
    // Allows external modules (like main.cpp for highlighting) to read the book data.
    const std::vector<std::vector<Move>>& getVariations();

    // <<< NEW: Declarations for helper functions >>>
    // Converts internal Move struct to algebraic string (e.g., "a1b2")
    std::string moveToAlgebraic(const Move& move);
    // Converts algebraic string (e.g., "a1b2") to internal Move struct
    // Throws std::invalid_argument on parsing error.
    Move algebraicToMove(const std::string& algNote);

    // Note: checkVariationExists was removed as its logic is now internal to saveVariation

} // namespace Book


