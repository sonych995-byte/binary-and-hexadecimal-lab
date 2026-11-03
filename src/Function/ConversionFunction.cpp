#include "CommonInclude.hpp"

int BinToDec(const std::string& input_bin) {

    int r = 0;

    for (char bit : input_bin) {
        int b = bit - '0';       // แปลงอักขระ '0' หรือ '1' ให้เป็นตัวเลข 0 หรือ 1
        
        r = (r << 1) + b;        // Horner's Method: เลื่อนบิตไปทางซ้าย (คูณ 2) แล้วบวกบิตถัดไป
    }

    return r;
}


std::string DecToBin(const std::string& input_dec) {

    if ( input_dec == "0" ) { return "0"; }

    int dec;
    std::vector<int> remainder;

    for ( char x : input_dec ) {
        dec = dec + ( x - '0');
    }

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

std::string DecToHex(const std::string& input_dec);
std::string BinToHex(const std::string& input_bin);
std::string HexToBin(const std::string& input_hex);
int BinToOct(const std::string& input_bin);
std::string OctToBin(const std::string& input_oct);
int HexToOct(const std::string& input_hex);