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


	struct Data {
		int value;
		int array[8];
		unsigned long millisTime;
	};

	typedef	struct Nodo {
		Data data;
		struct Nodo* siguente;
	}Nodo;

	typedef	struct Lista {
		Nodo* cabeza;
		int longitud;
	} Lista;

	Lista listaTerminal;

	Nodo* CrearNodo(Data data);
	void DestruirNodo(Nodo* nodo);
	int listaLongitud(Lista* lista);
	int listaVacia(Lista* lista);
	void RecorrerLista(Lista* lista);
	void InsertarPrincipio(Lista* lista, Data data);
	void InsertarFinal(Lista* lista, Data data);
	void EliminarPrincipio(Lista* lista);
	void EliminarUltimo(Lista* lista);

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

	void guardarDatosTerminal();
	void recorrerDatosTerminal();
	void borrarPrimerElemento();
	void borrarUltimoElemento();

};

#endif /* MUSIC_SOURCE_TERMINAL_H_ */

