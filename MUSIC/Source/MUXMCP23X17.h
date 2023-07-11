/*
 * MUXMCP23X17.h
 *
 *  Created on: 11 jul. 2023
 *      Author: Eduardo
 */

#ifndef SOURCE_MUXMCP23X17_H_
#define SOURCE_MUXMCP23X17_H_

#include "Arduino.h"
#include <Adafruit_MCP23X17.h>



class MUXMCP23X17 : public Adafruit_MCP23X17 {
public:

   //MUXMCP23X17(SemaphoreHandle_t* semaphorePtr) : Adafruit_MCP23X17(), semaphorePtr(semaphorePtr) {}

/*
   void setSemaphore(SemaphoreHandle_t semaphorePtr) {
      semaphore = semaphorePtr;
   }
*/
	void digitalWrite(uint8_t pin, uint8_t value, SemaphoreHandle_t* semaphore) {

	if (xSemaphoreTake(*semaphore, portMAX_DELAY) == pdTRUE) {
			// Sección crítica protegida por el semáforo


			// Llamada a la función original de digitalWrite
			//Adafruit_MCP23XXX::digitalWrite(pin, value);

		mcp.digitalWrite(pin, value);
			// Liberar el semáforo
			xSemaphoreGive(*semaphore);
		}

	}
/*
	uint8_t digitalRead(uint8_t pin){
		if (xSemaphoreTake(semaphore, portMAX_DELAY) == pdTRUE) {
			// Sección crítica protegida por el semáforo

			// Llamada a la función original de digitalRead
			Adafruit_MCP23XXX::digitalRead(pin);

			// Liberar el semáforo
			xSemaphoreGive(semaphore);
		}
	}
*/
	/*
private:
	 SemaphoreHandle_t semaphore;

*/
};
#endif /* SOURCE_MUXMCP23X17_H_ */
