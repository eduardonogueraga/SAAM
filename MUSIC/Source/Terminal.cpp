/*
 * Terminal.cpp
 *
 *  Created on: 8 jul. 2023
 *      Author: isrev
 *
 *
 *  Esta clase se encarga de almacenar los datos
 *  relativos a los terminales conectados a la linea
 *
 */

#include "Terminal.h"

Terminal::Terminal(char* nombreTerminal, byte numFotoSensor, byte numLineasCtl, byte numSensores) {

	strncpy(TERMINAL_NAME, nombreTerminal, sizeof(TERMINAL_NAME) - 1);
	TERMINAL_NAME[sizeof(TERMINAL_NAME) - 1] = '\0';  // Asegurarse de que el arreglo tenga un carácter nulo al final

	this->NUM_FOTO_SENSOR = numFotoSensor;
	this->NUM_LINEAS_CTL = numLineasCtl;
	this->NUM_SENSORES = numSensores;

	this->DATOS_FOTOSENSOR = 0;
	this->BAD_REPLY_STRIKE = 0;
	this->NO_REPLY_STRIKE = 0;
}

byte Terminal::getDatosFotosensor() const {
	return DATOS_FOTOSENSOR;
}


byte Terminal::getNumFotoSensor() const {
	return NUM_FOTO_SENSOR;
}


byte Terminal::getNumLineasCtl() const {
	return NUM_LINEAS_CTL;
}


byte Terminal::getNumSensores() const {
	return NUM_SENSORES;
}


const char* Terminal::getTerminalName() const {
	return TERMINAL_NAME;
}

void Terminal::setDatosFotosensor(byte datosFotosensor) {
	DATOS_FOTOSENSOR = datosFotosensor;
}

unsigned int Terminal::getBadReplyStrike() const {
	return BAD_REPLY_STRIKE;
}


unsigned int Terminal::getNoReplyStrike() const {
	return NO_REPLY_STRIKE;
}

void Terminal::addBadReplyStrike() {
	BAD_REPLY_STRIKE = BAD_REPLY_STRIKE++;
}

void Terminal::addNoReplyStrike() {
	NO_REPLY_STRIKE = NO_REPLY_STRIKE++;
}


unsigned int Terminal::getBadCommStrike() const {
	return BAD_COMM_STRIKE;
}

void Terminal::addBadCommStrike() {
	BAD_COMM_STRIKE = BAD_COMM_STRIKE++;
}

void Terminal::limpiarStrikes(){
	BAD_REPLY_STRIKE = 0;
	NO_REPLY_STRIKE = 0;
	BAD_COMM_STRIKE = 0;
}
