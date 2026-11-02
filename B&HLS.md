# 📘 DATA SHEET (ฉบับละเอียด)
## โปรแกรม Binary & Hexadecimal Laboratory — CLI / C++

> เอกสารนี้ครอบคลุม: สถาปัตยกรรมโปรแกรม, **API Reference แบบละเอียด**, **สูตร Encode/Decode ทุกตัว**, องค์ความรู้เชิงทฤษฎี, การจัดการ Error และตัวอย่างการใช้งานจริง

---

# PART 1 — สถาปัตยกรรมโปรแกรม (Architecture)

## 1.1 Module Diagram

```
┌──────────────────────────────────────────────────────────────┐
│                         main.cpp                             │
│                     (Menu Loop / CLI)                        │
└──────────────┬───────────────────────────────────────────────┘
               │ เรียกใช้
    ┌──────────┼────────────────┬───────────────┬───────────────┐
    ▼          ▼                ▼               ▼               ▼
┌─────────┐ ┌──────────┐  ┌──────────┐  ┌────────────┐  ┌──────────┐
│converter│ │ bitwise  │  │ validator│  │  formatter │  │ history  │
│  .h/.cpp│ │  .h/.cpp │  │  .h/.cpp │  │   .h/.cpp  │  │  .h/.cpp │
└─────────┘ └──────────┘  └──────────┘  └────────────┘  └──────────┘
     │           │              │               │              │
     └───────────┴──────────────┴───────────────┴──────────────┘
                                  │
                        ┌─────────▼─────────┐
                        │   types.h /       │
                        │   config.h        │
                        │ (WordSize, Base,  │
                        │  Signedness)      │
                        └───────────────────┘
```

## 1.2 ชนิดข้อมูลที่ใช้เป็น "ตัวกลาง" (Core Type)

```cpp
// ใช้ uint64_t เป็น storage ภายในเสมอ แล้วค่อย mask ตาม wordSize
// เหตุผล: bitwise ทำงานกับชนิดเดียวกันได้ง่าย และขยาย/ลดขนาดได้ด้วย mask
using RawValue = uint64_t;
```

## 1.3 โครงสร้าง Config (Global State)

```cpp
struct Config {
    int  wordSize   = 16;      // 8 | 16 | 32 | 64
    bool isSigned   = false;   // false = Unsigned, true = Signed (Two's Complement)
    int  displayBase= 16;      // 2 | 8 | 10 | 16 (ค่าที่ใช้แสดงผลเริ่มต้น)
};
extern Config g_cfg;           // global instance (หรือส่งเป็นพารามิเตอร์)
```

---

# PART 2 — API Reference (ละเอียดทุกฟังก์ชัน)

> **Convention**
> - ทุกฟังก์ชันที่รับ/คืนค่าเป็น "ตัวเลข" จะรับ/คืน `RawValue` (uint64_t) เสมอ
> - ค่าที่เกิน `wordSize` จะถูก **mask** ไว้ที่ฟังก์ชันผู้เรียก (ดู `maskFor()`)
> - ฟังก์ชันที่ Parse input จะ **throw `std::invalid_argument`** เมื่อเจอรูปแบบผิด
> - `wordSize` ต้องเป็น 8, 16, 32 หรือ 64 เท่านั้น มิฉะนั้น throw `std::invalid_argument`

---

## 2.1 Module: `config` / `limits`

### `unsigned long long maskFor(int wordSize)`

| รายการ | รายละเอียด |
|--------|-----------|
| **จุดประสงค์** | คืน bitmask ที่มีบิต 1 จำนวน `wordSize` บิต |
| **พารามิเตอร์** | `wordSize` ∈ {8,16,32,64} |
| **คืนค่า** | `(1<<wordSize) - 1` (หรือ `~0ULL` เมื่อ `wordSize == 64`) |
| **สูตร** | `mask = 2^wordSize - 1` |
| **Error** | `wordSize` ไม่ถูกต้อง → `std::invalid_argument` |

```cpp
unsigned long long maskFor(int wordSize) {
    if (wordSize == 64) return ~0ULL;
    if (wordSize != 8 && wordSize != 16 && wordSize != 32)
        throw std::invalid_argument("wordSize must be 8/16/32/64");
    return (1ULL << wordSize) - 1;
}
```
**ตัวอย่าง:**
```cpp
maskFor(8)  → 0x0000_0000_0000_00FF
maskFor(16) → 0x0000_0000_0000_FFFF
maskFor(32) → 0x0000_0000_FFFF_FFFF
maskFor(64) → 0xFFFF_FFFF_FFFF_FFFF
```

---

### `unsigned long long maxUnsigned(int wordSize)`
**สูตร:** `2^wordSize - 1`

| wordSize | maxUnsigned |
|----------|-------------|
| 8 | 255 |
| 16 | 65,535 |
| 32 | 4,294,967,295 |
| 64 | 18,446,744,073,709,551,615 |

---

### `long long minSigned(int wordSize)`
**สูตร:** `-2^(wordSize-1)`

| wordSize | minSigned |
|----------|-----------|
| 8 | −128 |
| 16 | −32,768 |
| 32 | −2,147,483,648 |
| 64 | −9,223,372,036,854,775,808 |

---

### `long long maxSigned(int wordSize)`
**สูตร:** `2^(wordSize-1) - 1`

| wordSize | maxSigned |
|----------|-----------|
| 8 | 127 |
| 16 | 32,767 |
| 32 | 2,147,483,647 |
| 64 | 9,223,372,036,854,775,807 |

---

## 2.2 Module: `validator` / `digit`

### `int digitValue(char c)`

| รายการ | รายละเอียด |
|--------|-----------|
| **จุดประสงค์** | แปลง character 1 ตัวเป็นค่าตัวเลข 0–15 |
| **พารามิเตอร์** | `c` ∈ `'0'..'9'`, `'A'..'F'`, `'a'..'f'` |
| **คืนค่า** | ค่าตัวเลข 0–15 |
| **Error** | character ไม่ใช่ตัวเลข/hex → `std::invalid_argument` |

**สูตร / Mapping:**
```
'0'..'9'  → 0..9           (c - '0')
'A'..'F'  → 10..15         (c - 'A' + 10)
'a'..'f'  → 10..15         (c - 'a' + 10)
```

```cpp
int digitValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    throw std::invalid_argument(std::string("bad digit: ") + c);
}
```

---

### `char valueDigit(int v, bool uppercase = true)`

**สูตร / Mapping (ย้อนกลับ):**
```
0..9    → '0'..'9'          ('0' + v)
10..15  → 'A'..'F' / 'a'..'f'  ('A' + (v-10)) หรือ ('a' + (v-10))
```

**ตัวอย่าง:** `valueDigit(10, true) → 'A'`, `valueDigit(15, false) → 'f'`

---

### `bool isValidBinary(const std::string& s)`
- **เงื่อนไข:** `s` ไม่ว่าง AND ทุก char เป็น `'0'` หรือ `'1'`
- **สูตร:** `!s.empty() && all_of(s, c -> c=='0'||c=='1')`

### `bool isValidHex(const std::string& s)`
- **เงื่อนไข:** `s` ไม่ว่าง AND ทุก char อยู่ใน `0-9A-Fa-f`

### `bool isValidDecimal(const std::string& s)`
- **เงื่อนไข:** `s` ไม่ว่าง AND (มี optional `'-'` นำหน้าได้ถ้า signed) AND ตัวที่เหลือเป็น `'0'..'9'`

### `bool fitsInWord(RawValue value, int wordSize, bool isSigned)`
**สูตร:**
- ถ้า **Unsigned**: `0 ≤ value ≤ 2^wordSize - 1`
- ถ้า **Signed (ค่าดิบเป็น bit-pattern)**: ทุก bit-pattern ใน n บิตเป็นได้หมด → ตรวจแค่ `value ≤ mask`
- ถ้า **Signed (ค่าที่ผู้ใช้กรอกเป็นเลขฐานสิบติดลบ)**: `-2^(n-1) ≤ value ≤ 2^(n-1) - 1`

```cpp
bool fitsInWord(RawValue v, int n, bool isSigned) {
    if (!isSigned) return v <= maxUnsigned(n);
    return v <= (RawValue)maxUnsigned(n); // เมื่อ v มาจาก bit-pattern
}
```

---

## 2.3 Module: `converter` (หัวใจของโปรแกรม)

### 🔹 `unsigned long long binaryToDecimal(const std::string& binary)`

| รายการ | รายละเอียด |
|--------|-----------|
| **จุดประสงค์** | แปลงเลขฐาน 2 (string) → ค่าตัวเลข |
| **พารามิเตอร์** | `binary` = string ของ `'0'/'1'` ความยาว 1..64 |
| **คืนค่า** | `RawValue` |
| **Error** | รูปแบบผิด / ยาวเกิน 64 → `std::invalid_argument` |

**สูตร (Horner's Method):**
$$D = \sum_{i=0}^{n-1} b_i \cdot 2^{n-1-i}, \quad b_i \in \{0,1\}$$

**อัลกอริทึม (loop):**
```
result = 0
for each char c in binary (ซ้าย→ขวา):
    result = result * 2 + (c - '0')
return result
```

**ตัวอย่าง:**
```
binaryToDecimal("1011") = 1*8 + 0*4 + 1*2 + 1*1 = 11
```

---

### 🔹 `std::string decimalToBinary(RawValue value, int wordSize)`

| รายการ | รายละเอียด |
|--------|-----------|
| **จุดประสงค์** | แปลงค่าตัวเลข → binary string **แบบ pad เต็ม wordSize บิต** |
| **พารามิเตอร์** | `value` = ค่าดิบ (จะถูก mask), `wordSize` |
| **คืนค่า** | string ยาว = `wordSize` ตัวอักษร (`'0'/'1'`) |
| **Error** | `wordSize` ผิด → throw |

**สูตร (Division-Remainder / หารสั้น):**
$$v_0 = value, \quad r_k = v_k \bmod 2, \quad v_{k+1} = \lfloor v_k / 2 \rfloor$$
ทำจนกว่า $v_k = 0$ แล้วนำ $r_k$ มาเรียงย้อนกลับ และ pad '0' ทางซ้ายให้ครบ `wordSize`

**อัลกอริทึม:**
```
value = value & maskFor(wordSize)      // mask ก่อน
s = "" 
for i = wordSize-1 downto 0:
    s += ((value >> i) & 1) ? '1' : '0'
return s
```
**หมายเหตุ:** วิธีนี้เร็วกว่าเพราะตรง ๆ กับบิต

**ตัวอย่าง (wordSize=8):**
```
decimalToBinary(11, 8)  = "00001011"
decimalToBinary(255, 8) = "11111111"
```

---

### 🔹 `std::string decimalToHex(RawValue value, int wordSize, bool uppercase = true)`

| รายการ | รายละเอียด |
|--------|-----------|
| **คืนค่า** | hex string ยาว = `wordSize / 4` ตัวอักษร |
| **สูตร** | เหมือน decimalToBinary แต่ group 4 บิต = 1 hex digit |

**สูตร:**
$$\text{hex digit}_k = \left\lfloor \frac{value}{16^k} \right\rfloor \bmod 16, \quad k = 0,\ldots,\frac{n}{4}-1$$

**อัลกอริทึม:**
```
value = value & maskFor(wordSize)
digits = wordSize / 4
s = ""
for i = digits-1 downto 0:
    nibble = (value >> (i*4)) & 0xF
    s += valueDigit(nibble, uppercase)
return s
```

**ตัวอย่าง (wordSize=16):**
```
decimalToHex(0xABCD, 16) = "ABCD"
decimalToHex(42, 8)      = "2A"
```

---

### 🔹 `unsigned long long hexToDecimal(const std::string& hex)`

**สูตร:**
$$D = \sum_{i=0}^{m-1} \text{digitValue}(h_i) \cdot 16^{m-1-i}$$

**อัลกอริทึม:**
```
result = 0
for each char c in hex:
    result = result * 16 + digitValue(c)
return result
```

**ตัวอย่าง:**
```
hexToDecimal("2F")  = 2*16 + 15 = 47
hexToDecimal("ABCD")= 10*4096 + 11*256 + 12*16 + 13 = 43981
```

---

### 🔹 `std::string binaryToHex(const std::string& binary, bool uppercase = true)`

**หลักการ:** 1 hex digit = 4 บิต → **group จากขวา** ถ้าไม่ครบ 4 ให้ pad `'0'` ทางซ้าย

**สูตร:**
$$\text{pad}(bin) = \underbrace{0\ldots0}_{p} \, bin, \quad p = (4 - (|bin| \bmod 4)) \bmod 4$$
แล้วแปลงทีละ 4 บิต

**อัลกอริทึม:**
```
pad = (4 - (binary.length() % 4)) % 4
binary = string(pad, '0') + binary
result = ""
for i = 0; i < binary.length(); i += 4:
    nibble = 0
    for j in 0..3:
        nibble = nibble*2 + (binary[i+j]-'0')
    result += valueDigit(nibble, uppercase)
return result
```

**ตัวอย่าง:**
```
binaryToHex("1011")         = "B"
binaryToHex("11111010")     = "FA"
binaryToHex("1011011")      → pad → "01011011" → "5B"
```

---

### 🔹 `std::string hexToBinary(const std::string& hex, int wordSize)`

**หลักการ:** แต่ละ hex digit → 4 บิต (Mapping Table คงที่)

**Mapping Table:**
| Hex | Binary | Hex | Binary |
|-----|--------|-----|--------|
| 0 | 0000 | 8 | 1000 |
| 1 | 0001 | 9 | 1001 |
| 2 | 0010 | A | 1010 |
| 3 | 0011 | B | 1011 |
| 4 | 0100 | C | 1100 |
| 5 | 0101 | D | 1101 |
| 6 | 0110 | E | 1110 |
| 7 | 0111 | F | 1111 |

**อัลกอริทึม:**
```
result = ""
for each char c in hex:
    v = digitValue(c)
    for i = 3 downto 0:
        result += ((v >> i) & 1) ? '1' : '0'
if (result.length() > wordSize) throw overflow;
while (result.length() < wordSize) result = '0' + result;  // pad
return result
```

**ตัวอย่าง:**
```
hexToBinary("FA", 8)  = "11111010"
hexToBinary("5", 16)  = "0000000000000101"
```

---

### 🔹 `std::string decimalToOctal(RawValue value, int wordSize)` *(optional)*
**สูตร:** หารด้วย 8 ซ้ำ (เหมือน binary/hex แต่ตัวหาร = 8)

---

### 🔹 `long long interpretSigned(RawValue raw, int wordSize)`

| รายการ | รายละเอียด |
|--------|-----------|
| **จุดประสงค์** | ตีความ bit-pattern เป็นจำนวนเต็ม **Signed (Two's Complement)** |
| **สูตร** | $\text{signed} = u - 2^n \cdot \lfloor 2u / 2^n \rfloor$ หรือถ้า $u < 2^{n-1}$ → $u$; อย่างอื่น $u - 2^n$ |

**สูตรชัดเจน:**
$$\text{signBit} = (raw \gg (n-1)) \ \&\ 1$$
$$\text{signed} = \begin{cases} raw & \text{signBit} = 0 \\ raw - 2^n & \text{signBit} = 1 \end{cases}$$

**อัลกอริทึม:**
```cpp
long long interpretSigned(RawValue raw, int n) {
    raw &= maskFor(n);
    RawValue signBit = (raw >> (n-1)) & 1ULL;
    if (!signBit) return (long long)raw;
    return (long long)(raw - (1ULL << n));   // หรือ raw - 2^n
}
```
**ตัวอย่าง (n=8):**
```
interpretSigned(0b00001011, 8) = 11
interpretSigned(0b11111011, 8) = 251 - 256 = -5
interpretSigned(0b10000000, 8) = 128 - 256 = -128
```

---

### 🔹 `RawValue encodeSigned(long long value, int wordSize)`

| รายการ | รายละเอียด |
|--------|-----------|
| **จุดประสงค์** | แปลงจำนวนเต็มลบ (หรือบวก) → bit-pattern ตาม Two's Complement |
| **สูตร** | $\text{encode}(v) = v \bmod 2^n$ (ในทางคณิตศาสตร์) = ถ้า $v \ge 0$ → $v$; ถ้า $v < 0$ → $2^n + v$ |

**สูตร:**
$$\text{encode}(v) = \begin{cases} v & v \ge 0 \\ 2^n + v & v < 0 \end{cases} = v \ \& \ (2^n - 1)$$

**อัลกอริทึม:**
```cpp
RawValue encodeSigned(long long value, int n) {
    return (RawValue)value & maskFor(n);   // bitwise AND ใช้ได้ทั้งบวก/ลบ
}
```
**ตัวอย่าง (n=8):**
```
encodeSigned(-5, 8)   = -5 & 0xFF = 0xFB = 251
encodeSigned(127, 8)  = 0x7F = 127
encodeSigned(-128, 8) = 0x80 = 128
```

**หมายเหตุทางคณิตศาสตร์:** ในภาษา C++ การ cast `long long` (ลบ) ไป `uint64_t` ให้ผลเป็น Two's Complement อยู่แล้ว (standard กำหนดไว้) ดังนั้น mask ตาม `wordSize` ก็พอ

---

### 🔹 `RawValue applyTwoComplement(RawValue raw, int wordSize)` *(ทางเลือก)*
สำหรับกรณีที่ผู้ใช้ขอ "กลับเครื่องหมาย" ของ bit-pattern:
**สูตร:** $\text{neg}(x) = ((\sim x) + 1) \ \& \ \text{mask}$
```cpp
RawValue applyTwoComplement(RawValue x, int n) {
    return ((~x) + 1) & maskFor(n);
}
```

---

## 2.4 Module: `bitwise`

> ทุกฟังก์ชันจะ **mask input และ output** ตาม `wordSize` เพื่อให้ทำงานเหมือนฮาร์ดแวร์จริง

### `RawValue opAND(RawValue a, RawValue b, int n)`
**สูตร:** `result = (a & b) & mask(n)`
**ตารางความจริง (ต่อ 1 บิต):**
| a | b | a&b |
|---|---|-----|
| 0 | 0 | 0 |
| 0 | 1 | 0 |
| 1 | 0 | 0 |
| 1 | 1 | 1 |

**ตัวอย่าง (n=8):** `0xF0 & 0x3C = 0x30` → binary: `11110000 & 00111100 = 00110000`

---

### `RawValue opOR(RawValue a, RawValue b, int n)`
**สูตร:** `result = (a | b) & mask(n)`
| a | b | a\|b |
|---|---|-----|
| 0 | 0 | 0 |
| 0 | 1 | 1 |
| 1 | 0 | 1 |
| 1 | 1 | 1 |

**ตัวอย่าง:** `0xF0 | 0x0F = 0xFF`

---

### `RawValue opXOR(RawValue a, RawValue b, int n)`
**สูตร:** `result = (a ^ b) & mask(n)`
| a | b | a^b |
|---|---|-----|
| 0 | 0 | 0 |
| 0 | 1 | 1 |
| 1 | 0 | 1 |
| 1 | 1 | 0 |

**คุณสมบัติที่ใช้บ่อย:**
- `a ^ a = 0`
- `a ^ 0 = a`
- `(a ^ b) ^ b = a` → ใช้ Swap โดยไม่ต้องใช้ temp

---

### `RawValue opNOT(RawValue a, int n)`
**สูตร:** `result = (~a) & mask(n) = (2^n - 1) - (a & mask(n))`
```cpp
RawValue opNOT(RawValue a, int n) {
    return (~a) & maskFor(n);
}
```
**ตัวอย่าง (n=8):**
```
opNOT(0b00001111) = 0b11110000 = 0xF0
opNOT(0)          = 0xFF
```

**หมายเหตุ:** ถ้าไม่ mask, `~a` ของ `uint64_t` จะ flip ถึง 64 บิต ซึ่งไม่ตรงกับ wordSize ที่ตั้งไว้ → **ต้อง mask เสมอ**

---

### `RawValue opSHL(RawValue a, int shift, int n)`
**สูตร:** `result = (a << shift) & mask(n)` ≈ $(a \cdot 2^{shift}) \bmod 2^n$

**ตัวอย่าง (n=8):**
```
opSHL(0b0001, 3, 8) = 0b1000 = 8        (1 × 2³ = 8)
opSHL(0x80, 1, 8)   = 0x00              (overflow หลุด wordSize)
```

**⚠️ ระวัง:** `shift ≥ 64` เป็น **undefined behavior** ใน C++ → ต้องเช็คก่อน:
```cpp
if (shift < 0 || shift >= 64) throw std::invalid_argument("shift out of range");
```

---

### `RawValue opSHR(RawValue a, int shift, int n)` — Logical Right Shift
**สูตร:** `result = (a >> shift) & mask(n)` ≈ $\lfloor a / 2^{shift} \rfloor$

**ตัวอย่าง (n=8):**
```
opSHR(0b1000, 3, 8) = 0b0001
opSHR(0xF0, 4, 8)   = 0x0F
```

---

### `RawValue opSAR(RawValue a, int shift, int n)` — Arithmetic Right Shift
**หลักการ:** คง sign bit ไว้ (เฉพาะเมื่อตีความเป็น signed)

**สูตร:**
$$\text{SAR}(a, s) = \begin{cases} a \gg s & \text{if } MSB(a) = 0 \\ (a \gg s) \ \big| \ (\text{mask\_high}) & \text{if } MSB(a) = 1 \end{cases}$$
โดย `mask_high = mask(n) & ~mask(n-s)` (บิตบนสุด n-s ตัวเป็น 1)

**อัลกอริทึม:**
```cpp
RawValue opSAR(RawValue a, int s, int n) {
    a &= maskFor(n);
    RawValue sign = (a >> (n-1)) & 1ULL;
    RawValue r = a >> s;
    if (sign) {
        RawValue highMask = maskFor(n) ^ maskFor(n - s);
        r |= highMask;
    }
    return r & maskFor(n);
}
```
**ตัวอย่าง (n=8):**
```
opSAR(0b10000000, 1, 8) = 0b11000000 = 0xC0   (signed -128/2 = -64)
opSHR(0b10000000, 1, 8) = 0b01000000 = 0x40   (unsigned 128/2 = 64)
```

---

## 2.5 Module: `formatter` / `display`

### `std::string formatValue(RawValue v, int base, int n, bool uppercase = true)`
- `base = 2` → เรียก `decimalToBinary(v, n)`
- `base = 8` → `decimalToOctal(v, n)`
- `base = 10` → ถ้า `g_cfg.isSigned` → `std::to_string(interpretSigned(v, n))`; ไม่ → `std::to_string(v)`
- `base = 16` → `decimalToHex(v, n, uppercase)`

### `void printBitGrouped(const std::string& bits)` — แสดงบิตแบบอ่านง่าย
**สูตรแบ่งกลุ่ม:** ทุก 4 บิตใส่ช่องว่าง จากขวาไปซ้าย

**ตัวอย่าง:** `"101110101010"` → `"1011 1010 1010"`

---

## 2.6 Module: `history`

```cpp
struct HistoryEntry {
    std::string operation;   // "AND", "DEC→HEX", ...
    std::string input;       // raw input ที่ผู้ใช้กรอก
    std::string output;      // ผลลัพธ์ที่แสดง
};

void pushHistory(const HistoryEntry& e);   // append
void displayHistory();                     // print ทั้งหมด
void clearHistory();                       // ล้าง
std::size_t historySize();
```

**Implementation:** `std::vector<HistoryEntry>` (Static หรือ Singleton)

---

## 2.7 Module: `cli`

### `int readMenuChoice(int min, int max)`
- อ่าน int จาก `std::cin`
- ถ้า `cin.fail()` → `cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');` → return `-1` (หรือ throw)
- ตรวจ `min ≤ choice ≤ max`

### `std::string readString(const std::string& prompt)`
- `std::getline(std::cin, s)` (ต้องระวัง `\n` ที่ค้างจาก `>>` ก่อนหน้า → ใช้ `cin.ignore()`)

### `RawValue readRawValue(const std::string& prompt, int base, int wordSize, bool isSigned)`
- อ่าน string → ตรวจ format ตาม base → เรียก converter
- ถ้า signed + base 10 + ค่าติดลบ → เรียก `encodeSigned()`
- return RawValue (พร้อม mask)

---

# PART 3 — สูตร Encode/Decode (ฉบับรวม)

## 3.1 ตาราง Base Conversion ครบทุกทิศทาง

| จาก → ไป | สูตร / Algorithm |
|----------|------------------|
| Bin → Dec | $D = \sum b_i \cdot 2^{n-1-i}$ (Horner: `r = r*2 + b`) |
| Dec → Bin | หารสั้นด้วย 2, เก็บเศษ, เรียงย้อนกลับ (หรือ `(v>>i)&1`) |
| Hex → Dec | $D = \sum h_i \cdot 16^{m-1-i}$ (Horner: `r = r*16 + h`) |
| Dec → Hex | หารสั้นด้วย 16, เก็บเศษ, แปลงเป็น A-F |
| Bin → Hex | Group 4 บิตจากขวา → mapping table |
| Hex → Bin | แต่ละ hex digit → 4 บิต |
| Bin → Oct | Group 3 บิตจากขวา |
| Oct → Bin | แต่ละ oct digit → 3 บิต |
| Hex → Oct | ผ่าน Dec เป็นตัวกลาง (หรือผ่าน Bin) |

## 3.2 Two's Complement (Signed Encode/Decode)

**Encode (Signed → Bit-pattern):**
$$\text{encode}(v, n) = v \ \& \ (2^n - 1)$$
ใช้ได้ทั้ง $v \ge 0$ และ $v < 0$ (C++ กำหนดพฤติกรรมของ signed → unsigned conversion ไว้)

**Decode (Bit-pattern → Signed):**
$$\text{decode}(u, n) = \begin{cases} u & u < 2^{n-1} \\ u - 2^n & u \ge 2^{n-1} \end{cases}$$

**Alternative (encode จากการกลับบิต — เพื่อการสอน):**
$$\text{encode}(-x) = ((\sim x) + 1) \ \& \ \text{mask}$$

## 3.3 Bitwise Operation Formulas (ฉบับ mask)

| Operation | Formula |
|-----------|---------|
| AND | `(a & b) & M` |
| OR | `(a \| b) & M` |
| XOR | `(a ^ b) & M` |
| NOT | `((~a) & M)` = `M - (a & M)` |
| SHL | `(a << s) & M` |
| SHR (logical) | `(a & M) >> s` |
| SAR (arithmetic) | `(a & M) >> s \| (MSB ? highMask : 0)` |
| Rotate Left | `((a << s) \| (a >> (n - s))) & M` |
| Rotate Right | `((a >> s) \| (a << (n - s))) & M` |

**โดยที่:** `M = maskFor(n)`, `highMask = M ^ maskFor(n-s)`

## 3.4 Range Check Formulas

| ประเภท | Min | Max |
|--------|-----|-----|
| Unsigned | 0 | $2^n - 1$ |
| Signed | $-2^{n-1}$ | $2^{n-1} - 1$ |

**การตรวจ overflow หลังการดำเนินการ:**
- Unsigned ADD: ถ้า `a + b > maxUnsigned(n)` → overflow
- Signed ADD: ถ้า `sign(a) == sign(b)` และ `sign(result) != sign(a)` → overflow
- Signed MUL: ตรวจตามช่วง minSigned / maxSigned

---

# PART 4 — ความรู้เชิงทฤษฎีที่ต้องใช้ (พร้อมสูตร)

## 4.1 ระบบเลขฐาน

### สูตรทั่วไป (Positional Number System)
ค่าในฐาน $b$:
$$N = \sum_{i=0}^{k-1} d_i \cdot b^i, \quad d_i \in \{0, 1, \ldots, b-1\}$$

**ตัวอย่าง:** $1101_2 = 1\cdot 2^3 + 1\cdot 2^2 + 0\cdot 2^1 + 1\cdot 2^0 = 13_{10}$

### Horner's Method (การแปลงเป็นฐานสิบแบบเร็ว)
$$r_0 = d_0; \quad r_{i+1} = r_i \cdot b + d_{i+1}; \quad N = r_{k-1}$$

### Division-Remainder (การแปลงจากฐานสิบเป็นฐาน $b$)
$$q_0 = N, \quad d_i = q_i \bmod b, \quad q_{i+1} = \lfloor q_i / b \rfloor$$
ทำจน $q_k = 0$ แล้ว $N = (d_{k-1} \ldots d_1 d_0)_b$

## 4.2 Two's Complement — ทฤษฎีเต็ม

**นิยาม:** สำหรับ n บิต ค่าลบ $-x$ (โดย $x > 0$) แทนด้วย
$$[-x]_n = 2^n - x$$

**คุณสมบัติสำคัญ:**
- มีศูนย์เพียงค่าเดียว ($+0$ เท่านั้น ไม่มี $-0$)
- ช่วงค่า: $[-2^{n-1}, 2^{n-1}-1]$
- การบวก/ลบทำงานเหมือน unsigned ภายใน $\bmod 2^n$
- บิตซ้ายสุด (MSB) เป็น sign bit

**การพิสูจน์สูตร Encode:**
$$-x = (\sim x) + 1 \text{ ใน n บิต}$$
เพราะ $\sim x = (2^n - 1) - x$ แล้ว $(\sim x) + 1 = 2^n - x$ ✓

## 4.3 Bitwise Algebra

**Laws (กฎพีชคณิตของบิต):**
| Law | สูตร |
|-----|------|
| Identity | `a \| 0 = a`, `a & M = a`, `a ^ 0 = a` |
| Annihilator | `a \| M = M`, `a & 0 = 0` |
| Idempotent | `a \| a = a`, `a & a = a` |
| Involution | `~~a = a` (ในขอบเขต n บิต) |
| De Morgan | `~(a & b) = ~a \| ~b`, `~(a \| b) = ~a & ~b` |
| XOR swap | `a ^= b; b ^= a; a ^= b;` |

**การใช้งานจริง:**
- **Mask:** `x & 0x0F` → เอา 4 บิตล่าง
- **Set bit:** `x |= (1 << k)`
- **Clear bit:** `x &= ~(1 << k)`
- **Toggle bit:** `x ^= (1 << k)`
- **Test bit:** `(x >> k) & 1`

## 4.4 ขนาดข้อมูล (Data Type Sizes)

| Type | บิต | Unsigned Max | Signed Range |
|------|-----|--------------|--------------|
| `uint8_t` / `int8_t` | 8 | 255 | [-128, 127] |
| `uint16_t` / `int16_t` | 16 | 65,535 | [-32,768, 32,767] |
| `uint32_t` / `int32_t` | 32 | 4,294,967,295 | [-2,147,483,648, 2,147,483,647] |
| `uint64_t` / `int64_t` | 64 | 18.4 × 10¹⁸ | [-9.22 × 10¹⁸, 9.22 × 10¹⁸] |

## 4.5 C++ Standard Library ที่ใช้

| Header | Component | ใช้ทำอะไร |
|--------|-----------|-----------|
| `<cstdint>` | `uint8_t … uint64_t` | ชนิดข้อมูลขนาดคงที่ |
| `<limits>` | `std::numeric_limits<T>::max()` | ตรวจ overflow |
| `<string>` | `std::string`, `std::to_string`, `std::stoull` | จัดการ string |
| `<sstream>` | `std::stringstream` | แปลงตัวเลข ↔ string |
| `<iomanip>` | `std::setw`, `std::setfill` | format output |
| `<vector>` | `std::vector<HistoryEntry>` | history |
| `<stdexcept>` | `std::invalid_argument` | throw error |
| `<algorithm>` | `std::all_of`, `std::transform` | validation |

## 4.6 UB & Trap ที่ต้องหลีกเลี่ยง

| อันตราย | คำอธิบาย | วิธีป้องกัน |
|---------|----------|-------------|
| `a << s` เมื่อ `s ≥ 64` | Undefined behavior | `if (s < 0 \|\| s >= 64) throw` |
| `1 << 31` (ใน `int`) | UB เมื่อ signed overflow | ใช้ `1ULL << 31` |
| `(int)0xFFFFFFFF` | implementation-defined | ใช้ `uint32_t` |
| `~a` ของ 64-bit | ได้ 64 บิตเต็ม | mask ด้วย `maskFor(n)` |
| `cin >> x` แล้วไม่ตรวจ fail | input ค้าง | `if(cin.fail()) {cin.clear(); cin.ignore(...);}` |

---

# PART 5 — Flow ของ Menu + ตัวอย่างการใช้งาน

## 5.1 Main Menu Flow

```
┌─────────────────────────────────────────────┐
│  Binary & Hexadecimal Laboratory            │
├─────────────────────────────────────────────┤
│  Word Size: 16  |  Type: UNSIGNED           │
│  Display Base: HEX                          │
├─────────────────────────────────────────────┤
│  1) Convert Base                            │
│  2) Bitwise Operation                       │
│  3) Settings (Word Size / Signed)           │
│  4) Show History                            │
│  5) Clear History                           │
│  0) Exit                                    │
└─────────────────────────────────────────────┘
```

## 5.2 Conversion Sub-menu
```
1) Binary → Decimal
2) Decimal → Binary
3) Hex → Decimal
4) Decimal → Hex
5) Binary ↔ Hex
6) Two's Complement Calculator
0) Back
```

## 5.3 ตัวอย่าง Session (จริง)

```
> 1                         // Conversion
> 1                         // Bin → Dec
Enter binary: 11111010
Result: 250                 // Decimal
Binary : 1111 1010          // Grouped
Hex    : FA
Signed : -6 (if signed 8-bit)

> 2                         // Bitwise
> 1                         // AND
Enter A (hex): F0
Enter B (hex): 3C
Result (hex): 30
Result (bin): 0011 0000
```

## 5.4 Pseudocode ของ `main()`

```cpp
int main() {
    while (true) {
        printMainMenu();
        int c = readMenuChoice(0, 5);
        switch (c) {
            case 1: runConversionMenu(); break;
            case 2: runBitwiseMenu();    break;
            case 3: runSettingsMenu();   break;
            case 4: displayHistory();    break;
            case 5: clearHistory();      break;
            case 0: return 0;
        }
    }
}
```

---

# PART 6 — Error Handling / Validation Checklist

| Error Case | การจัดการ |
|-----------|-----------|
| Binary มีตัวอักษรอื่น | `isValidBinary()` → reject |
| Hex ยาวเกิน `wordSize/4` | เช็ค overflow ก่อน parse |
| Decimal ติดลบแต่ Unsigned | reject |
| Decimal เกิน `maxUnsigned/Signed` | reject + แสดงช่วงที่รับได้ |
| Shift ≥ wordSize | reject หรือ mask |
| `cin.fail()` | clear + ignore |
| `wordSize` ไม่ใช่ 8/16/32/64 | throw `invalid_argument` |
| ค่าว่าง (empty input) | reject |
| Overflow หลัง AND/OR | ไม่เกิด (AND/OR ปลอดภัย) |
| Overflow หลัง SHL | ตรวจหลังทำ mask |
| Overflow หลัง Signed ADD | ตรวจ sign flag |

**Error Message Template:**
```
[ERROR] Invalid <field>: <reason>
        Expected: <spec>
        Range   : <min>..<max>
```

---

# PART 7 — File Structure แนะนำ

```
binary-hex-lab/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── config.h              // Config struct + g_cfg
│   ├── types.h               // RawValue typedef
│   ├── limits.h / .cpp       // maskFor, max/min
│   ├── validator.h / .cpp    // isValid*, digitValue, valueDigit
│   ├── converter.h / .cpp    // binaryToDecimal, decimalToHex, ...
│   ├── bitwise.h / .cpp      // opAND, opOR, ...
│   ├── formatter.h / .cpp    // formatValue, printBitGrouped
│   ├── history.h / .cpp      // HistoryEntry + push/display
│   └── cli.h / .cpp          // menu functions
└── tests/
    ├── test_converter.cpp    // unit tests (assert)
    ├── test_bitwise.cpp
    └── test_signed.cpp
```

**CMakeLists.txt (ย่อ):**
```cmake
cmake_minimum_required(VERSION 3.15)
project(binary_hex_lab CXX)
set(CMAKE_CXX_STANDARD 17)
add_executable(lab
    src/main.cpp
    src/limits.cpp
    src/validator.cpp
    src/converter.cpp
    src/bitwise.cpp
    src/formatter.cpp
    src/history.cpp
    src/cli.cpp)
```

---

# PART 8 — Unit Test Cases ที่ควรมี (สำหรับยืนยันสูตร)

| # | Function | Input | Expected |
|---|----------|-------|----------|
| 1 | `binaryToDecimal("1011")` | — | `11` |
| 2 | `decimalToBinary(11, 8)` | — | `"00001011"` |
| 3 | `decimalToHex(0xABCD, 16)` | — | `"ABCD"` |
| 4 | `hexToDecimal("FF")` | — | `255` |
| 5 | `binaryToHex("11111010")` | — | `"FA"` |
| 6 | `hexToBinary("5", 16)` | — | `"0000000000000101"` |
| 7 | `interpretSigned(0xFB, 8)` | — | `-5` |
| 8 | `encodeSigned(-5, 8)` | — | `0xFB` |
| 9 | `opAND(0xF0, 0x3C, 8)` | — | `0x30` |
| 10 | `opNOT(0x0F, 8)` | — | `0xF0` |
| 11 | `opSHL(0x80, 1, 8)` | — | `0x00` (overflow) |
| 12 | `opSAR(0x80, 1, 8)` | — | `0xC0` |
| 13 | `opSHR(0x80, 1, 8)` | — | `0x40` |
| 14 | `maskFor(16)` | — | `0xFFFF` |
| 15 | `maxSigned(8)` | — | `127` |

---

## สรุปสั้น ๆ

| ส่วน | สาระสำคัญ |
|------|-----------|
| **API** | ทุกฟังก์ชันรับ/คืน `RawValue` (uint64_t) และรับ `wordSize` เสมอ เพื่อ mask ให้ตรงขนาดจริง |
| **Encode/Decode** | ใช้สูตร **Horner** (→ Dec) และ **Division-Remainder** (← Dec); Two's Complement ใช้ `v & mask` (encode) และ `u - 2^n` (decode) |
| **Bitwise** | ทุก operation ต้อง `& maskFor(n)` เพื่อจำลองฮาร์ดแวร์ n บิต; ระวัง `shift ≥ 64` เป็น UB |
| **Validation** | ตรวจ format, range, overflow และ `cin.fail()` ทุกจุด |
| **Knowledge** | Number Systems, Two's Complement, Bitwise Algebra, ขนาดข้อมูล, C++ `<cstdint>` / `<limits>` / `<sstream>` |