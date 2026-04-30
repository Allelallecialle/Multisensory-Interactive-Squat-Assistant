#include <unity.h>

#include "Adafruit_BNO055.h"
#include "EEPROM.h"
#include "config.h"
#include "utils.h"

#include <cstring>

// Helper: write an arbitrary calibration pattern and an ID to EEPROM at offset.
static adafruit_bno055_offsets_t make_pattern(uint16_t seed) {
    adafruit_bno055_offsets_t d{};
    d.accel_offset_x = seed + 1;
    d.accel_offset_y = seed + 2;
    d.accel_offset_z = seed + 3;
    d.mag_offset_x   = seed + 4;
    d.mag_offset_y   = seed + 5;
    d.mag_offset_z   = seed + 6;
    d.gyro_offset_x  = seed + 7;
    d.gyro_offset_y  = seed + 8;
    d.gyro_offset_z  = seed + 9;
    d.accel_radius   = seed + 10;
    d.mag_radius     = seed + 11;
    return d;
}

static bool offsets_equal(const adafruit_bno055_offsets_t& a,
                          const adafruit_bno055_offsets_t& b) {
    return std::memcmp(&a, &b, sizeof(a)) == 0;
}

void setUp(void) {
    EEPROM.clear();
    reset_calibration = false;
    display_BNO055_info = false;  // silence stub Serial calls
}

void tearDown(void) {}

// Test 1 — fresh EEPROM (ID slot is 0, sensor reports a real ID):
// no calibration data should be loaded into the BNO; instead a fresh
// calibration is captured from the chip and persisted to EEPROM.
void test_fresh_eeprom_writes_new_calibration(void) {
    Adafruit_BNO055 bno(1, BNO055_ADDRESS_A, &Wire);
    bno.mock_sensor_id = 42;
    bno.mock_fully_calibrated = true;
    bno.mock_returned_offsets = make_pattern(100);

    long bnoID = 0;
    calibrateIMU(bno, 0, bnoID);

    TEST_ASSERT_EQUAL_INT32(42, bnoID);
    TEST_ASSERT_FALSE_MESSAGE(bno.set_offsets_called,
        "no prior calibration in EEPROM, must not call setSensorOffsets");
    TEST_ASSERT_TRUE_MESSAGE(bno.ext_crystal_set,
        "external crystal must be enabled");
    TEST_ASSERT_EQUAL_INT(1, bno.get_offsets_call_count);

    long stored_id = 0;
    EEPROM.get(0, stored_id);
    TEST_ASSERT_EQUAL_INT32_MESSAGE(42, stored_id,
        "sensor ID must be persisted at EEPROM offset 0");

    adafruit_bno055_offsets_t stored_cal{};
    EEPROM.get((int)sizeof(long), stored_cal);
    TEST_ASSERT_TRUE_MESSAGE(offsets_equal(stored_cal, bno.mock_returned_offsets),
        "captured calibration must be persisted right after the ID");
}

// Test 2 — matching ID in EEPROM: the saved offsets are pushed back into
// the BNO via setSensorOffsets, and EEPROM is left untouched at the ID slot.
void test_matching_id_loads_calibration_from_eeprom(void) {
    const long stored_id = 7;
    const adafruit_bno055_offsets_t stored_cal = make_pattern(500);
    EEPROM.put(0, stored_id);
    EEPROM.put((int)sizeof(long), stored_cal);

    Adafruit_BNO055 bno(2, BNO055_ADDRESS_A, &Wire);
    bno.mock_sensor_id = 7;
    bno.mock_mag_cal   = 3;  // performMagCal exits immediately

    long bnoID = 0;
    calibrateIMU(bno, 0, bnoID);

    TEST_ASSERT_EQUAL_INT32(7, bnoID);
    TEST_ASSERT_TRUE_MESSAGE(bno.set_offsets_called,
        "matching ID must trigger setSensorOffsets with the saved data");
    TEST_ASSERT_TRUE_MESSAGE(offsets_equal(bno.last_offsets_set, stored_cal),
        "offsets pushed to chip must equal what was in EEPROM");

    long unchanged = 0;
    EEPROM.get(0, unchanged);
    TEST_ASSERT_EQUAL_INT32_MESSAGE(7, unchanged,
        "ID slot must not be rewritten when calibration was already valid");
}

// Test 3 — ID stored in EEPROM doesn't match the chip's sensor ID:
// behave like a fresh device — fresh calibration is captured and the ID
// slot is overwritten.
void test_mismatched_id_overwrites_with_fresh_calibration(void) {
    const long stale_id = 99;
    EEPROM.put(0, stale_id);
    EEPROM.put((int)sizeof(long), make_pattern(11));

    Adafruit_BNO055 bno(1, BNO055_ADDRESS_A, &Wire);
    bno.mock_sensor_id = 1;  // doesn't match stale_id
    bno.mock_fully_calibrated = true;
    bno.mock_returned_offsets = make_pattern(2000);

    long bnoID = 0;
    calibrateIMU(bno, 0, bnoID);

    TEST_ASSERT_FALSE_MESSAGE(bno.set_offsets_called,
        "stale EEPROM data must not be loaded into the chip");

    long stored_id = 0;
    EEPROM.get(0, stored_id);
    TEST_ASSERT_EQUAL_INT32_MESSAGE(1, stored_id,
        "EEPROM ID slot must be overwritten with the live sensor ID");

    adafruit_bno055_offsets_t stored_cal{};
    EEPROM.get((int)sizeof(long), stored_cal);
    TEST_ASSERT_TRUE_MESSAGE(offsets_equal(stored_cal, bno.mock_returned_offsets),
        "fresh calibration must replace the stale data");
}

// Test 4 — two IMUs share one EEPROM. Verify that the addresses chosen
// in main.cpp/setup() (0 and sizeof(long) + sizeof(adafruit_bno055_offsets_t))
// don't make the second IMU's data overlap the first's. This is the layout
// the production code uses when both BNOs are enabled.
void test_two_imus_use_non_overlapping_eeprom_slots(void) {
    const int eeAddress_1 = 0;
    const int eeAddress_2 = sizeof(long) + sizeof(adafruit_bno055_offsets_t);

    Adafruit_BNO055 bno1(1, BNO055_ADDRESS_A, &Wire);
    bno1.mock_sensor_id = 1;
    bno1.mock_fully_calibrated = true;
    bno1.mock_returned_offsets = make_pattern(10);

    Adafruit_BNO055 bno2(2, BNO055_ADDRESS_B, &Wire2);
    bno2.mock_sensor_id = 2;
    bno2.mock_fully_calibrated = true;
    bno2.mock_returned_offsets = make_pattern(900);

    long id1 = 0, id2 = 0;
    calibrateIMU(bno1, eeAddress_1, id1);
    calibrateIMU(bno2, eeAddress_2, id2);

    long stored_id_1 = 0, stored_id_2 = 0;
    EEPROM.get(eeAddress_1, stored_id_1);
    EEPROM.get(eeAddress_2, stored_id_2);

    TEST_ASSERT_EQUAL_INT32_MESSAGE(1, stored_id_1,
        "IMU1 ID must remain after IMU2 calibration is written");
    TEST_ASSERT_EQUAL_INT32_MESSAGE(2, stored_id_2,
        "IMU2 ID must be at its dedicated offset");

    adafruit_bno055_offsets_t cal1{}, cal2{};
    EEPROM.get(eeAddress_1 + (int)sizeof(long), cal1);
    EEPROM.get(eeAddress_2 + (int)sizeof(long), cal2);

    TEST_ASSERT_TRUE_MESSAGE(offsets_equal(cal1, bno1.mock_returned_offsets),
        "IMU1 calibration must survive IMU2 write");
    TEST_ASSERT_TRUE_MESSAGE(offsets_equal(cal2, bno2.mock_returned_offsets),
        "IMU2 calibration must be at its own slot");
}

// Test 5 — reset_calibration path. Documents the expected contract: when
// reset_calibration is set, the function should still leave EEPROM in a
// consistent state with the live sensor ID at offset eeAddress and the
// fresh calibration at eeAddress + sizeof(long).
//
// NOTE: the current implementation in utils.cpp:194-201 increments
// eeAddress *before* the EEPROM.get(eeAddress, eeBnoID) read, so the ID
// is fetched from the calibration-data slot. As a result, the EEPROM
// layout after the function returns is shifted by sizeof(long). This
// test will FAIL until that branch is fixed (move the eeAddress mutation
// to after the ID/cal-data writes, or just zero out the ID slot).
void test_reset_calibration_keeps_layout_consistent(void) {
    EEPROM.put(0, (long)42);  // pretend a previous run left valid data
    EEPROM.put((int)sizeof(long), make_pattern(77));

    reset_calibration = true;

    Adafruit_BNO055 bno(1, BNO055_ADDRESS_A, &Wire);
    bno.mock_sensor_id = 42;
    bno.mock_fully_calibrated = true;
    bno.mock_returned_offsets = make_pattern(3000);

    long bnoID = 0;
    calibrateIMU(bno, 0, bnoID);

    long stored_id = 0;
    EEPROM.get(0, stored_id);
    TEST_ASSERT_EQUAL_INT32_MESSAGE(42, stored_id,
        "after reset+recalibration, ID must end up at offset 0 (not shifted)");

    adafruit_bno055_offsets_t stored_cal{};
    EEPROM.get((int)sizeof(long), stored_cal);
    TEST_ASSERT_TRUE_MESSAGE(offsets_equal(stored_cal, bno.mock_returned_offsets),
        "after reset+recalibration, fresh cal must be at offset sizeof(long)");
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_fresh_eeprom_writes_new_calibration);
    RUN_TEST(test_matching_id_loads_calibration_from_eeprom);
    RUN_TEST(test_mismatched_id_overwrites_with_fresh_calibration);
    RUN_TEST(test_two_imus_use_non_overlapping_eeprom_slots);
    RUN_TEST(test_reset_calibration_keeps_layout_consistent);
    return UNITY_END();
}
