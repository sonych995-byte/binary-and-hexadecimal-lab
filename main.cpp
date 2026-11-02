#include "src/include/CommonInclude.hpp"

void clear() {
    system("clear");
}


void UI() {

    std::cout << "┌─────────────────────────────────────────────┐\n";
    std::cout << "│  Binary & Hexadecimal Laboratory            │\n";
    std::cout << "├─────────────────────────────────────────────┤\n";
    std::cout << "│  Word Size: 16  |  Type: UNSIGNED           │\n";
    std::cout << "│  Display Base: HEX                          │\n";
    std::cout << "├─────────────────────────────────────────────┤\n";
    std::cout << "│  1) Convert Base                            │\n";
    std::cout << "│  2) Bitwise Operation                       │\n";
    std::cout << "│  3) Settings (Word Size / Signed)           │\n";
    std::cout << "│  4) Show History                            │\n";
    std::cout << "│  5) Clear History                           │\n";
    std::cout << "│  0) Exit                                    │\n";
    std::cout << "└─────────────────────────────────────────────┘\n";

}

void ConversionSubUI() {

    std::cout << "┌─────────────────────────────────────────────┐\n";
    std::cout << "│  Binary & Hexadecimal Laboratory            │\n";
    std::cout << "├─────────────────────────────────────────────┤\n";
    std::cout << "│  Word Size: 16  |  Type: UNSIGNED           │\n";
    std::cout << "│  Display Base: HEX                          │\n";
    std::cout << "├─────────────────────────────────────────────┤\n";
    std::cout << "│  1) Binary → Decimal                        │\n";
    std::cout << "│  2) Decimal → Binary                        │\n";
    std::cout << "│  3) Hex → Decimal                           │\n";
    std::cout << "│  4) Decimal → Hex                           │\n";
    std::cout << "│  5) Binary ↔ Hex                            │\n";
    std::cout << "|  6) Two's Complement Calculator             |\n";            
    std::cout << "│  0) Back                                    │\n";
    std::cout << "└─────────────────────────────────────────────┘\n";

}

int main() {
    
    while (true) {

        UI();
        std::string mode;
        std::cout << "Enter mode: ";
        std::cin >> mode;

        if ( mode == "1" ) {
            clear();
            ConversionSubUI();

            std::string ConversionMode;
            std::cout << "Enter conversionmode: ";

            if ( ConversionMode == "1" )
        }

    }



    return 0;
}