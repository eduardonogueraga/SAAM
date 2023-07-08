/*
 * Terminal.h
 *
 *  Created on: 8 jul. 2023
 *      Author: isrev
 */

#ifndef MUSIC_SOURCE_TERMINAL_H_
#define MUSIC_SOURCE_TERMINAL_H_

#include "Arduino.h"

class Terminal {

private:
	char TERMINAL_NAME[10];
	byte NUM_SENSORES;
	byte NUM_LINEAS_CTL;
	byte NUM_FOTO_SENSOR;

	unsigned int NO_REPLY_STRIKE;
	unsigned int BAD_REPLY_STRIKE;
	unsigned int BAD_COMM_STRIKE;

	byte DATOS_FOTOSENSOR;

public:
	Terminal(char* nombreTerminal, byte numFotoSensor = 1, byte numLineasCtl = 2, byte numSensores = 8);

	byte getDatosFotosensor() const ;
	byte getNumFotoSensor() const;
	byte getNumLineasCtl() const;
	byte getNumSensores() const;
	const char* getTerminalName() const;
	unsigned int getBadReplyStrike() const;
	unsigned int getNoReplyStrike() const;
	unsigned int getBadCommStrike() const;

	void setDatosFotosensor(byte datosFotosensor);

	void addBadReplyStrike();
	void addNoReplyStrike();
	void addBadCommStrike();
	void limpiarStrikes();

};

#endif /* MUSIC_SOURCE_TERMINAL_H_ */

