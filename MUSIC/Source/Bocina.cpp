/*
 * Bocina.cpp
 *
 *  Created on: 26 jun. 2020
 *      Author: isrev
 */

#include "Bocina.h"
#include "Arduino.h"

Bocina::Bocina()
{

}

void Bocina::sonarBocina(unsigned short intervalo) {

	if((millis()-bocinaTiempoAnt>=intervalo) && (mcp.digitalRead(BOCINA_PIN)==1)){
		Serial.println("BOCINA OUT");
		mcp.digitalWrite(BOCINA_PIN,LOW);
		bocinaTiempoAnt=millis();
	}

	if((millis()-bocinaTiempoAnt>=intervalo) && (mcp.digitalRead(BOCINA_PIN)==0)){
		Serial.println("BOCINA IN");

		mcp.digitalWrite(BOCINA_PIN,HIGH);
		bocinaTiempoAnt=millis();
	}
}

void Bocina::stopBocina(){
	mcp.digitalWrite(BOCINA_PIN,LOW);
}

void Bocina::bocinaAlert(){
	mcp.digitalWrite(BOCINA_PIN, HIGH);
	delay(1000);
	mcp.digitalWrite(BOCINA_PIN,LOW);
}


