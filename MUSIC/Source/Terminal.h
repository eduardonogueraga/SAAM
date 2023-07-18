/*
 * Terminal.h
 *
 *  Created on: 8 jul. 2023
 *      Author: isrev
 */

#ifndef MUSIC_SOURCE_TERMINAL_H_
#define MUSIC_SOURCE_TERMINAL_H_

#define MAX_NODOS_EN_EJECUCION 20
#define TIEMPO_COMBO 700 //Tiempo para que se considere que un salto es consecutivo en el tiempo
#define TIEMPO_VIDA_NODO 25000

#define UMBRAL_NO_REPLY_STRIKE 5
#define UMBRAL_BAD_REPLY_STRIKE 10
#define UMBRAL_BAD_COMM_STRIKE 10
#define UMBRAL_FOTORESISTENCIA 150
#define UMBRAL_PERSISTENCIA_FOTORESISTENCIA 3

#include "Arduino.h"
#include "Macros.h"

class Terminal {

private:
	char TERMINAL_NAME[10];
	byte NUM_SENSORES;
	byte NUM_LINEAS_CTL;
	byte NUM_FOTO_SENSOR;

	unsigned int NO_REPLY_STRIKE;
	unsigned int BAD_REPLY_STRIKE;
	unsigned int BAD_COMM_STRIKE;

	const double UMBRAL_SENSOR_INDIVIDUAL = 99;
	const double UMBRAL_SENSOR_TOTAL = 200;
	const double PORCENTAJE_SALTO_UNITARIO = 4.0;


	int DATOS_FOTOSENSOR;
	byte datosControlLineas[MAX_DATOS_CTL_LINEA];


	struct Data {
		byte sampleSensores[MAX_DATOS_SUB_TRAMA];
		unsigned long marcaTiempo;
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
	double calcularPorcentaje(int nSaltos, int nMatch, int maxMatchConsecutivo);
	double EvaluarSensor(Lista* lista, int numSensor);
	void purgarNodosViejos(Lista* lista);

	byte nodosRevisados = 0;
	byte persistenciaFotoresistencia = 0;

public:
	Terminal(char* nombreTerminal, byte numFotoSensor = 1, byte numLineasCtl = 2, byte numSensores = 8);

	InterpretacionTerminal interpretacion;

	int getDatosFotosensor() const ;
	byte getNumFotoSensor() const;
	byte getNumLineasCtl() const;
	byte getNumSensores() const;
	const char* getTerminalName() const;
	unsigned int getBadReplyStrike() const;
	unsigned int getNoReplyStrike() const;
	unsigned int getBadCommStrike() const;


	void setDatosFotosensor(int datosFotosensor);

	void addBadReplyStrike();
	void addNoReplyStrike();
	void addBadCommStrike();
	void limpiarStrikes();

	void guardarDatosTerminal(byte* arrSensorSamples, byte* arrControlLineas);
	void recorrerDatosTerminal();
	void borrarPrimerElemento();
	void borrarUltimoElemento();

	InterpretacionTerminal evaluarDatosTerminal();

};

#endif /* MUSIC_SOURCE_TERMINAL_H_ */

