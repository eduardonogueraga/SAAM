/*
 * Utiles.h
 *
 *  Created on: 3 sept. 2023
 *      Author: Eduardo
 *
 *      Funciones generalistas de la alarma
 */

#ifndef SOURCE_ALARMAUTIL_UTILES_H_
#define SOURCE_ALARMAUTIL_UTILES_H_

//MANEJO DE CADENAS

String fixedLengthString(String& original, size_t fixedLength) {
	if (original.length() < fixedLength) {
		byte espacios = fixedLength - original.length();
		for (int i = 0;   i < espacios; ++  i) {
			original += " ";
		}
		return original;
	}
	else {
		return original.substring(0, fixedLength);
	}
}


//MANEJO DE MEMORIA NVS
uint8_t leerFlagEE(const char* key) {

	NVSMemory.begin("SAA_DATA", false);
	uint8_t value = NVSMemory.getUChar(key);
	NVSMemory.end();

	return value;
}

int leerFlagEEInt(const char* key) {

	NVSMemory.begin("SAA_DATA", false);
	int value = NVSMemory.getInt(key);
	NVSMemory.end();

	return value;
}


void guardarFlagEE(const char* key, uint8_t value) {

	NVSMemory.begin("SAA_DATA", false);
	uint8_t previo = NVSMemory.getUChar(key);

	if(previo != value || value == NULL){
		NVSMemory.putUChar(key, value);
	}

	NVSMemory.end();
}


void guardarFlagEE(const char* key, int value) {

	NVSMemory.begin("SAA_DATA", false);
	int previo = NVSMemory.getUChar(key);

	if(previo != value || value == NULL){
		NVSMemory.putInt(key, value);
	}

	NVSMemory.end();
}


String leerCadenaEE(const char* key) {
	NVSMemory.begin("SAA_DATA", false);
	String value = NVSMemory.getString(key);
	NVSMemory.end();

	return value;
}


void guardarCadenaEE(const char* key, String* value) {

	NVSMemory.begin("SAA_DATA", false);
	String previo = NVSMemory.getString(key);

	if(previo.isEmpty() || previo != *value){
		NVSMemory.putString(key, *value);
	}

	NVSMemory.end();
}


template <typename T> void NVS_SaveData(const char* key, T value) {

	NVSMemory.begin("SAA_DATA", false);
	NVSMemory.putBytes(key, &value, sizeof(value));
	NVSMemory.end();
}

template <typename T> T NVS_RestoreData(const char* key) {

	NVSMemory.begin("SAA_DATA", false);
	T value; //Struct generico

	NVSMemory.getBytes(key, &value, sizeof(value));
	NVSMemory.end();

	// Devolvemos el struct leído
	return value;
}


#endif /* SOURCE_ALARMAUTIL_UTILES_H_ */
