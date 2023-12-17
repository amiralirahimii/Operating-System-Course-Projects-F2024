#include <ostream>
#include <string>

const std::string ANSI_RED = "\x1B[31m";
const std::string ANSI_GRN = "\x1B[32m";
const std::string ANSI_YEL = "\x1B[33m";
const std::string ANSI_BLU = "\x1B[34m";
const std::string ANSI_MAG = "\x1B[35m";
const std::string ANSI_CYN = "\x1B[36m";
const std::string ANSI_WHT = "\x1B[37m";
const std::string ANSI_RST = "\x1B[0m";
const int ANSI_LEN = 9;

enum class Color {
    BLK = 30,
    RED = 31,
    GRN = 32,
    YEL = 33,
    BLU = 34,
    MAG = 35,
    CYN = 36,
    WHT = 37,
    DEF = 39,
    RST = 0,
};

inline std::ostream& operator<<(std::ostream& os, Color clr) {
    return os << "\x1B[" << static_cast<int>(clr) << 'm';
}