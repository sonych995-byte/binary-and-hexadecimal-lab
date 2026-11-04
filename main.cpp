#include "src/include/CommonInclude.hpp"
#include "src/Function/ConversionFunction.cpp"

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
        clear();
        UI();
        std::string mode;
        std::cout << "Enter mode: ";
        std::cin >> mode;

        if ( mode == "1" ) {
            clear();
            ConversionSubUI();

            std::string ConversionMode;
            std::cout << "Enter conversionmode: ";
            std::cin >> ConversionMode;

            if ( ConversionMode == "1" ) {
                clear;
                ConversionSubUI;

                std::string input_bin;
                std::cout << "Enter binary: ";
                std::cin >> input_bin;

                int result = BinToDec(input_bin);

                std::cout << "\nResult: " << std::to_string(result);

                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                std::cin.get();

            } else if ( ConversionMode == "2" ) {
                clear;
                ConversionSubUI;

                std::string input_dec;
                std::cout << "Enter decimal: ";
                std::cin >> input_dec;

                std::string result = DecToBin(std::stoi(input_dec));

                std::cout << "\nResult: " << result;

                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                std::cin.get();
            } else if ( ConversionMode == "3" ) {
                clear;
                ConversionSubUI;

                std::string input_hex;
                std::cout << "Enter hexadecimal: ";
                std::cin >> input_hex;

                int result = HexToDec(input_hex);

                std::cout << "\nResult: " << std::to_string(result);

                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                std::cin.get();
            } else if ( ConversionMode == "4" ) {
                clear;
                ConversionSubUI;

                std::string input_dec;
                std::cout << "Enter decimal: ";
                std::cin >> input_dec;

                std::string result = DecToHex(std::stoi(input_dec));

                std::cout << "\nResult: " << result;

                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                std::cin.get();
            } else if ( ConversionMode == "5" ) {
                clear;
                ConversionSubUI;

                std::string input_bin;
                std::cout << "Enter binary: ";
                std::cin >> input_bin;

                std::string result = BinToHex(input_bin;

                std::cout << "\nResult: " << result;

                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                std::cin.get();
        }

    }



    return 0;
}