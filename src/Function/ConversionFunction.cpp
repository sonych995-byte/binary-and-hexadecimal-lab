#include "../include/CommonInclude.hpp"

int BinToDec(const std::string& input_bin) {

    int r = 0;

    for (char bit : input_bin) {
        int b = bit - '0';       // แปลงอักขระ '0' หรือ '1' ให้เป็นตัวเลข 0 หรือ 1
        
        r = (r << 1) + b;        // Horner's Method: เลื่อนบิตไปทางซ้าย (คูณ 2) แล้วบวกบิตถัดไป
    }

    return r;
}

std::string DecToBin( int& input_dec) {

    if ( input_dec == 0 ) { return "0"; }

    int dec = input_dec;
    std::vector<int> remainder;

    while ( dec > 0 ) {
        remainder.push_back( dec % 2 );
        dec = dec / 2;
    }

    std::reverse(remainder.begin(), remainder.end());

    std::string result;

    for ( int bit : remainder ) {
        char bin = bit + '0';

        result += bin;
    }

    return result;
    
}

int HexToDec(const std::string& input_hex) {

    std::vector<int> parser_hex;
    std::vector<char> hex = {'A', 'B', 'C', 'D', 'E', 'F'};

    for ( char x : input_hex ) {

        if (std::ranges::contains(hex, x)) {
            if ( x == 'A' ) {
                parser_hex.push_back(10);
            } else if ( x == 'B' ) {
                parser_hex.push_back(11);
            } else if ( x == 'C' ) {
                parser_hex.push_back(12);
            } else if ( x == 'D' ) {
                parser_hex.push_back(13);
            } else if ( x == 'E' ) {
                parser_hex.push_back(14);
            } else if ( x == 'F' ) {
                parser_hex.push_back(15);
            } else {
                continue;
            }
        } else {
            int y = x - '0';
            parser_hex.push_back(y);
        }
    
    }

    int decimal_result = 0;
    int power = 1; // เริ่มจาก 16^0 = 1

    // วิ่งลูปถอยหลัง จากหลักหน่วยไปจนถึงหลักซ้ายสุด
    for (int i = parser_hex.size() - 1; i >= 0; --i) {
        decimal_result += parser_hex[i] * power;
        power *= 16; // เพิ่มพลังคูณ 16 เท่าในหลักถัดไป
    }

    return decimal_result;
  
}

std::string DecToHex( int& input_dec) {

    std::vector<int> remainders;

    while ( input_dec > 0 ) {
        remainders.push_back( input_dec % 16 );
        input_dec = input_dec / 16;
    }

    std::reverse(remainders.begin(), remainders.end());

    std::string result;

    for ( int remainder : remainders) {

        if ( remainder == 10 ) {
            result += "A";
        } else if ( remainder == 11 ) {
            result += "B";
        } else if ( remainder == 12 ) {
            result +=  "C";
        } else if ( remainder == 13 ) {
            result += "D";
        } else if ( remainder == 14 ) {
            result += "E";
        } else if ( remainder == 15 ) {
            result += "F";
        } else {
            std::string x = std::to_string(remainder);
            result += x;
        }
    }

    return result;

}

std::string DecToOct( int& input_dec) {

    if ( input_dec == 0 ) { return "0"; }

    int dec = input_dec;
    std::vector<int> remainder;

    while ( dec > 0 ) {
        remainder.push_back( dec % 8 );
        dec = dec / 8;
    }

    std::reverse(remainder.begin(), remainder.end());

    std::string result;

    for ( int bit : remainder ) {
        char oct = bit + '0';

        result += oct;
    }

    return result;

}

int OctToDec( std::string& input_oct) {


    std::string oct = input_oct;

    std::reverse(oct.begin(), oct.end());

    std::vector<int> multiple;
    int row = 0;

    for ( char bit : oct ) {
        bit = bit - '0';

        multiple.push_back( bit * pow(8, row));

        ++row;
    }

    int result;

    for (int x : multiple ) {
        result = result + x;
    }

    return result;
}

std::string BinToHex(const std::string& input_bin) {

    int dec = BinToDec(input_bin);
    std::string hex = DecToHex(dec);
    return hex;
}

std::string HexToBin(const std::string& input_hex) {

    int dec = HexToDec(input_hex);
    std::string bin = DecToBin(dec);
    return bin;

}

std::string BinToOct(const std::string& input_bin) {

    int dec = BinToDec(input_bin);
    std::string oct = DecToOct(dec);
    return oct;

}

std::string OctToBin( std::string& input_oct) {

    int dec = OctToDec(input_oct);
    std::string bin = DecToBin(dec);
    return bin;
}

std::string HexToOct( std::string& input_hex) {

    int dec = HexToDec(input_hex);
    std::string oct = DecToOct(dec);
    return oct;
}