
#include "new_http.h"
#include "../obk_config.h"

#if ENABLE_HA_DISCOVERY

#include "../cJSON/cJSON.h"
#include "../new_pins.h"
#include "../mqtt/new_mqtt.h"
#include "../cmnds/cmd_public.h"

typedef enum {
	/// @brief Switch
	RELAY = 0,

	/// @brief 
	LIGHT_ON_OFF,

	/// @brief Single PWM
	LIGHT_PWM,

	/// @brief 2 PWM setup (brightness and temperature)
	LIGHT_PWMCW,

	/// @brief RGB (3 PWM)
	LIGHT_RGB,

	/// @brief RGB + temperature (5 PWM or LED driver)
	LIGHT_RGBCW,

	/// @brief Power sensors (voltage, current, power)
	ENERGY_METER_SENSOR,

	POWER_SENSOR,

	/// @Brief Binary Sensor
	BINARY_SENSOR,

	/// @brief Temperature sensor
	TEMPERATURE_SENSOR,
	/// @brief Humidity sensor
	HUMIDITY_SENSOR,

	/// @brief Battery level sensor in perc, under battery topic
	BATTERY_SENSOR,
	/// @brief Battery votage sensor in mV
	BATTERY_VOLTAGE_SENSOR,

	/// @brief CO2 sensor in ppm
	CO2_SENSOR,
	/// @brief TVOC sensor in ppb
	TVOC_SENSOR,

	/// @brief 
	VOLTAGE_SENSOR,
	/// @brief 
	CURRENT_SENSOR,
	/// @brief 
	//POWER_SINGLE_SENSOR,
	/// @brief 
	POWERFACTOR_SENSOR,
	/// @brief 
	FREQUENCY_SENSOR,
	/// @brief 
	CUSTOM_SENSOR,
	/// @brief 
	SMOKE_SENSOR,
	/// @brief 
	READONLYLOWMIDHIGH_SENSOR,
	// lx unit
	ILLUMINANCE_SENSOR,
	/// @brief °C unit
	HASS_TEMP,
	/// @brief dBm unit
	HASS_RSSI,
	/// @brief Time firmware is alive in secs
	HASS_UPTIME,
	/// @brief Firmware build info
	HASS_BUILD,
	/// @brief 
	HASS_SSID,
	/// @brief 
	HASS_IP,
	/// @brief Wh, kWh
	ENERGY_SENSOR,
	// hPa
	PRESSURE_SENSOR,
	/// @Brief Timestamp Sensor
	TIMESTAMP_SENSOR,
	// Ph
	WATER_QUALITY_PH,
	// ORP
	WATER_QUALITY_ORP,
	// TDS
	WATER_QUALITY_TDS,
	/// @brief Battery level sensor in perc, under channel topic
	BATTERY_CHANNEL_SENSOR,
	HASS_HVAC,
	HASS_FAN,
	HASS_SELECT,
	HASS_PERCENT,
} ENTITY_TYPE;

//unique_id is defined in hass_populate_unique_id and is based on CFG_GetDeviceName() whose size is CGF_DEVICE_NAME_SIZE.
//Sample unique_id would be deviceName_entityType_index.
//Currently supported entityType is `relay` or `light` - 5 char.
#define HASS_UNIQUE_ID_SIZE     (CGF_DEVICE_NAME_SIZE + 1 + 5 + 1 + 4)

//channel is based on unique_id (see hass_populate_device_config_channel)
#define HASS_CHANNEL_SIZE       (HASS_UNIQUE_ID_SIZE + 32)

//Size of JSON (1 less than MQTT queue holding)
#define HASS_JSON_SIZE          (MQTT_PUBLISH_ITEM_VALUE_LENGTH - 1)

/// @brief HomeAssistant device discovery information
typedef struct HassDeviceInfo_s {
	char unique_id[HASS_UNIQUE_ID_SIZE];
	char channel[HASS_CHANNEL_SIZE];
	char json[HASS_JSON_SIZE];

	cJSON* root;
	cJSON* device;
	cJSON* ids;
} HassDeviceInfo;

void hass_print_unique_id(http_request_t* request, const char* fmt, ENTITY_TYPE type, int index, int asensdatasetix);
HassDeviceInfo* hass_init_relay_device_info(int index, ENTITY_TYPE type, bool bInverse);
HassDeviceInfo* hass_init_device_info(ENTITY_TYPE type, int index, const char* payload_on, const char* payload_off, int asensdatasetix, const char *title);
HassDeviceInfo* hass_init_light_device_info(ENTITY_TYPE type);
HassDeviceInfo* hass_init_energy_sensor_device_info(int index, int asensdatasetix);
HassDeviceInfo* hass_init_light_singleColor_onChannels(int toggle, int dimmer, int brightness_scale);
HassDeviceInfo* hass_init_binary_sensor_device_info(int index, bool bInverse);
HassDeviceInfo* hass_init_sensor_device_info(ENTITY_TYPE type, int channel, int decPlaces, int decOffset, int divider);
HassDeviceInfo* hass_createHVAC(float min, float max, float step, const char **fanOptions, int numFanOptions,
	const char **swingOptions, int numSwingOptions, const char **swingHOptions, int numSwingHOptions);
HassDeviceInfo* hass_createFanWithModes(const char *label, const char *stateTopic,
	const char *command, const char **options, int numOptions);
HassDeviceInfo* hass_createSelectEntity(const char* state_topic, const char* command_topic, int numoptions,
	const char* options[], const char* title);
HassDeviceInfo* hass_createSelectEntityIndexed(const char* state_topic, const char* command_topic, int numoptions,
	const char* options[], const char* title);

HassDeviceInfo* hass_createToggle(const char *label, const char *stateTopic, const char *commandTopic);
const char* hass_build_discovery_json(HassDeviceInfo* info);
void hass_free_device_info(HassDeviceInfo* info); 
char *hass_generate_multiplyAndRound_template(int decimalPlacesForRounding, int decimalPointOffset, int divider);

#endif // ENABLE_HA_DISCOVERY
