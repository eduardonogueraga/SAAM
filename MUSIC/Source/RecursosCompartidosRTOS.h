/*
 * RecursosCompartidosRTOS.h
 *
 *  Created on: 12 jul. 2023
 *      Author: isrev
 */

#ifndef MUSIC_SOURCE_RECURSOSCOMPARTIDOSRTOS_H_
#define MUSIC_SOURCE_RECURSOSCOMPARTIDOSRTOS_H_

#include "Arduino.h"
#include <Adafruit_MCP23X17.h>

//extern Adafruit_MCP23X17 mcp; //Instancia compartida no acceder directamente

class RecursosCompartidosRTOS {
private:
	SemaphoreHandle_t* semaphorePtr;
	Adafruit_MCP23X17* mcpPtr;
public:

	//RecursosCompartidosRTOS(SemaphoreHandle_t* semaphorePtr) : semaphorePtr(semaphorePtr) {}

	 RecursosCompartidosRTOS(SemaphoreHandle_t* semaphorePtr, Adafruit_MCP23X17* mcp) {
	    this->semaphorePtr = semaphorePtr;
	    this->mcpPtr = mcp;
	  }


	void digitalWrite(uint8_t pin, uint8_t value) {

		Adafruit_MCP23X17 aux = *mcpPtr;

		if (xSemaphoreTake(*semaphorePtr, portMAX_DELAY) == pdTRUE) {
			// Llamada a la funcion original de digitalWrite
			aux.digitalWrite(pin, value);
			// Liberar el semaforo
			xSemaphoreGive(*semaphorePtr);
		}

	}


	uint8_t digitalRead(uint8_t pin){
		uint8_t value = 0;

		Adafruit_MCP23X17 aux = *mcpPtr;

		if (xSemaphoreTake(*semaphorePtr, portMAX_DELAY) == pdTRUE) {
			// Llamada a la funcion original de digitalRead
			value = aux.digitalRead(pin);
			// Liberar el semaforo
			xSemaphoreGive(*semaphorePtr);
		}

		return value;
	}

	void test(){


		if (xSemaphoreTake(*semaphorePtr, portMAX_DELAY) == pdTRUE) {


			Serial.print("hola: ");
			Serial.println(xPortGetCoreID());
			delay(1000);

			xSemaphoreGive(*semaphorePtr);
		}


	}

};

#endif /* MUSIC_SOURCE_RECURSOSCOMPARTIDOSRTOS_H_ */
