#ifndef ___LUNOKIOT__I2C__DB___
#define  ___LUNOKIOT__I2C__DB___

#ifdef __cplusplus
extern "C" {
#endif

// all know device addresses (and some kind of contract to develop those i2c drivers)
#define I2C_ADDR_BMM150 0x10
#define I2C_ADDR_MFRC522 0x28
#define I2C_ADDR_AXP192 0x34
#define I2C_ADDR_OLED_128x32 0x3C
#define I2C_ADDR_OLED_128x64 0x3D
#define I2C_ADDR_SHT30 0x44
#define I2C_ADDR_BM8563 0x51
#define I2C_ADDR_SGP30 0x58
#define I2C_ADDR_MPU6886 0x68
#define I2C_ADDR_BMP280 0x76

// https://static-cdn.m5stack.com/image/m5-docs_table/I2C_Address.pdf

/*
M5StickC/CPlus:
    this->DetectI2CHardware("Local", 21,22);
    this->DetectI2CHardware("Hy2.0-4P", 32,33); // Grove
    this->DetectI2CHardware("Hat", 0,26);
M5Atom & Matrix:
    this->DetectI2CHardware("Hy2.0-4P", 32,26); // Grove
Matrix:
    this->DetectI2CHardware("Local", 25,21); // https://docs.m5stack.com/en/core/atom_matrix
*/

// a simple empty declaration
extern const char *i2cDatabase[128];
// convenient function to fill the database, gcc 10.2.1 dont support designated initializers :(
void BuildI2CDatabase();

#ifdef __cplusplus
}
#endif

#endif // ___LUNOKIOT__I2C__DB___