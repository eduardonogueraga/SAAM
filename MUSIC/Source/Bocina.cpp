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

	if((millis()-bocinaTiempoAnt>=intervalo) && (rcomp1.digitalRead(BOCINA_PIN)==1)){
		Serial.println("BOCINA OUT");
		rcomp1.digitalWrite(BOCINA_PIN,LOW);
		bocinaTiempoAnt=millis();
	}

	if((millis()-bocinaTiempoAnt>=intervalo) && (rcomp1.digitalRead(BOCINA_PIN)==0)){
		Serial.println("BOCINA IN");

		rcomp1.digitalWrite(BOCINA_PIN,HIGH);
		bocinaTiempoAnt=millis();
	}
}

void Bocina::stopBocina(){
	rcomp1.digitalWrite(BOCINA_PIN,LOW);
}

void Bocina::bocinaAlert(){
	rcomp1.digitalWrite(BOCINA_PIN, HIGH);
	delay(1000);
	rcomp1.digitalWrite(BOCINA_PIN,LOW);
}


