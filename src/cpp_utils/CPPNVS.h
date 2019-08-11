/*
 * NVS.h
 *
 *  Created on: Mar 27, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_CPPNVS_H_
#define COMPONENTS_CPP_UTILS_CPPNVS_H_
#include <nvs.h>
#include <Arduino.h>

/**
 * @brief Provide Non Volatile Storage access.
 */
class NVS {
public:
	NVS();
	void init(String name, nvs_open_mode openMode = NVS_READWRITE);
	virtual ~NVS();
	void commit();

	void erase();
	void erase(String key);
	bool exists(String key);
	String get(String key, bool isBlob = false);
	int get(String key, uint8_t* result, size_t& length);
	int get(String key, uint32_t& value);
	void set(String key, String data, bool isBlob = false);
	void set(String key, uint32_t value);
	void set(String key, uint8_t* data, size_t length);

private:
	String m_name;
	nvs_handle m_handle;

};

#endif /* COMPONENTS_CPP_UTILS_CPPNVS_H_ */
