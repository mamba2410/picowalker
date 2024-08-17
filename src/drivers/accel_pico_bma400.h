#ifndef PW_DRIVER_ACCEL_BMA400_H
#define PW_DRIVER_ACCEL_BMA400_H

#define ACCEL_SCL_PIN       2 // GP 2, phys 4
#define ACCEL_MOSI_PIN      3 // GP 3, phys 5
#define ACCEL_MISO_PIN      4 // GP 4, phys 6
#define ACCEL_CS_PIN        20 // GP 20, phys 26
#define ACCEL_SPI_SPEED     1000000 // 1MHz

#define ACCEL_READ_MASK     0x80
#define ACCEL_WRITE_MASK    0x7f

#define ACCEL_REG_CHIPID    0x00
#define ACCEL_REG_ERR       0x02
#define ACCEL_REG_STATUS    0x03
#define ACCEL_REG_ACC_X_LSB 0x04
#define ACCEL_REG_ACC_X_MSB 0x05
#define ACCEL_REG_ACC_Y_LSB 0x06
#define ACCEL_REG_ACC_Y_MSB 0x07
#define ACCEL_REG_ACC_Z_LSB 0x08
#define ACCEL_REG_ACC_Z_MSB 0x09
#define ACCEL_REG_TEMP      0x11
#define ACCEL_REG_STEP_CNT_0    0x15
#define ACCEL_REG_STEP_CNT_1    0x16
#define ACCEL_REG_STEP_CNT_2    0x17
#define ACCEL_REG_STEP_STAT     0x18
#define ACCEL_REG_ACC_CONFIG0   0x19
#define ACCEL_REG_ACC_CONFIG1   0x1a
#define ACCEL_REG_ACC_CONFIG2   0x1b
#define ACCEL_REG_INT_CONFIG0   0x1f
#define ACCEL_REG_INT_CONFIG1   0x20
#define ACCEL_REG_INT1_MAP      0x21
#define ACCEL_REG_INT2_MAP      0x22
#define ACCEL_REG_INT12_MAP     0x23

#define ACCEL_REG_AUTOLOWPOW0   0x2b
#define ACCEL_REG_AUTOLOWPOW1   0x2c
#define ACCEL_REG_AUTOWAKEUP0   0x2d
#define ACCEL_REG_AUTOWAKEUP1   0x2e

#define STEP_CNT_INT_EN         0x01
#define STEP_STAT_MASK          0x03
#define STEP_STAT_STILL         0x00
#define STEP_STAT_WALKING       0x01
#define STEP_STAT_RUNNING       0x01

#define CHIP_ID                 0x90

#define ACCEL_POWER_MASK        0x03
#define ACCEL_POWER_OFFSET      1
#define ACCEL_POWER_SLEEP       0x00
#define ACCEL_POWER_LOW         0x01
#define ACCEL_POWER_NORMAL      0x02

#endif /* PW_DRIVER_ACCEL_BMA400_H */
