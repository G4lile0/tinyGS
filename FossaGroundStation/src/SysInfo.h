
struct SysInfo {
  float batteryChargingVoltage = 0.0f;
  float batteryChargingCurrent = 0.0f;
  float batteryVoltage = 0.0f;
  float solarCellAVoltage = 0.0f;
  float solarCellBVoltage = 0.0f;
  float solarCellCVoltage = 0.0f;
  float batteryTemperature = 0.0f;
  float boardTemperature = 0.0f;
  int mcuTemperature = 0;
  int resetCounter = 0;
  char powerConfig = 0b11111111;
};