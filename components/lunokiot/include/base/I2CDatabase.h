#ifndef ___LUNOKIOT__I2C__DB___
#define  ___LUNOKIOT__I2C__DB___

#define I2C_ADDR_BMM150 0x10
#define I2C_ADDR_MFRC522 0x28
#define I2C_ADDR_AXP192 0x34
#define I2C_ADDR_SHT30 0x44
#define I2C_ADDR_BM8563 0x51
#define I2C_ADDR_SGP30 0x58
#define I2C_ADDR_MPU6886 0x68
#define I2C_ADDR_BMP280 0x76

/* dfence/lunokIoT/lib/lunokIoT/OLD/device/M5StickCPlus/M5StickCPlusDevice.cpp
M5StickC
    this->DetectI2CHardware("Local", 21,22);
    this->DetectI2CHardware("Hy2.0-4P", 32,33); // Grove
    this->DetectI2CHardware("Hat", 0,26);
M5Atom/Matrix
    this->DetectI2CHardware("Hy2.0-4P", 32,26); // Grove
M5AtomMatrix
    this->DetectI2CHardware("Local", 25,21); // https://docs.m5stack.com/en/core/atom_matrix
*/
const char *i2cDatabase[128] = { nullptr };
//const char *i2cDatabase_Grove[128] = { nullptr };

// https://static-cdn.m5stack.com/image/m5-docs_table/I2C_Address.pdf
void BuildI2CDatabase() {
    i2cDatabase[I2C_ADDR_BMM150] = "BMM150";
    i2cDatabase[I2C_ADDR_MFRC522] = "MFRC522 (RFID)";
    i2cDatabase[I2C_ADDR_AXP192] = "AXP192 (PMU)";
    i2cDatabase[I2C_ADDR_SHT30] = "SHT30";
    i2cDatabase[I2C_ADDR_BM8563] = "BM8563 (RTC)";
    i2cDatabase[I2C_ADDR_SGP30] = "SGP30";
    
    i2cDatabase[I2C_ADDR_MPU6886] = "MPU6886 (MEMS)";
    i2cDatabase[I2C_ADDR_BMP280] = "BMP280";
}

#endif // ___LUNOKIOT__I2C__DB___