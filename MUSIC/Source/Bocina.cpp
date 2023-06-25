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

	//if((millis()-bocinaTiempoAnt>=intervalo) && (pcf8575.digitalRead(BOCINA_PIN)==1)){
	if((millis()-bocinaTiempoAnt>=intervalo) && (mcp.digitalRead(BOCINA_PIN)==1)){
		Serial.println("BOCINA OUT");

		//pcf8575.digitalWrite(BOCINA_PIN,LOW);
		mcp.digitalWrite(BOCINA_PIN,LOW);
		bocinaTiempoAnt=millis();
	}

	//if((millis()-bocinaTiempoAnt>=intervalo) && (pcf8575.digitalRead(BOCINA_PIN)==0)){
	if((millis()-bocinaTiempoAnt>=intervalo) && (mcp.digitalRead(BOCINA_PIN)==0)){
		Serial.println("BOCINA IN");

		//pcf8575.digitalWrite(BOCINA_PIN, HIGH);
		mcp.digitalWrite(BOCINA_PIN,HIGH);
		bocinaTiempoAnt=millis();
	}
}

void Bocina::stopBocina(){
	//pcf8575.digitalWrite(BOCINA_PIN,LOW);
	mcp.digitalWrite(BOCINA_PIN,LOW);
}

void Bocina::bocinaAlert(){
	//pcf8575.digitalWrite(BOCINA_PIN, HIGH);
	mcp.digitalWrite(BOCINA_PIN, HIGH);
	delay(1000);
	//pcf8575.digitalWrite(BOCINA_PIN,LOW);
	mcp.digitalWrite(BOCINA_PIN,LOW);
}


