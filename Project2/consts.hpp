#ifndef CONSTS_HPP_INCLUDE
#define CONSTS_HPP_INCLUDE

namespace consts {
    constexpr char BUILDING_EXE[] = "./building.out";
    constexpr char RESOURCE_EXE[] = "./resource.out";
    constexpr char BILLS_EXE[] = "./billsOffice.out";
    
    constexpr char PIPES_PATH[] = "namedpipes";
    constexpr char BUILDING_TO_BILL[] = "building_to_bill";
    constexpr char BILL_TO_BUILDING[] = "bill_to_building";
    constexpr char ELECTRICTY_FILE_NAME[] = "Electricity.csv";
    constexpr char GAS_FILE_NAME[] = "Gas.csv";
    constexpr char WATER_FILE_NAME[] = "Water.csv";
    constexpr char BILLS_FILE_NAME[] = "bills.csv";
    constexpr char ELECTRICTY_USER_INPUT[] = "Electricity";
    constexpr char GAS_USER_INPUT[] = "Gas";
    constexpr char WATER_USER_INPUT[] = "Water";
    constexpr int NUMBER_OF_RESOURSES = 3;
    constexpr int NUMBER_OF_DAY_IN_MONTH = 30;
    constexpr int NUMBER_OF_MONTH_IN_YEAR = 12;
    constexpr int NUMBER_OF_HOURS_IN_DAY = 6;
    constexpr int NUMBER_OF_SHAKHES = 6;
    const char INNER_PARSER = '#';
    const char OUTER_PARSER = '$';
    const char NAME_PARSER = '%';
    const char FINAL_PARSER = '^';
    // constexpr char MAIN_PROCESS[] = "main";
    constexpr int BUFFER_SIZE = 2048;
}

#endif
