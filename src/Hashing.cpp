#include "Hashing.h" // Include the header

namespace Zobrist {
    // Define the actual storage for the keys
    //vvv MODIFIED vvv --- Use new table name --- vvv
    std::vector<std::vector<std::vector<uint64_t>>> piecePlayerKeys;
    //^^^ MODIFIED ^^^----------------------------^^^
    uint64_t sideToMoveKey = 0;
    bool initialized = false;
}


