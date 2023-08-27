/*
 * Terminal.h
 *
 *  Created on: 8 jul. 2023
 *      Author: isrev
 */

#ifndef MUSIC_SOURCE_TERMINAL_H_
#define MUSIC_SOURCE_TERMINAL_H_

#define MAX_NODOS_EN_EJECUCION 20
#define TIEMPO_COMBO 40000 //Tiempo para que se considere que un salto es consecutivo en el tiempo
#define TIEMPO_VIDA_NODO 90000

#define UMBRAL_NO_REPLY_STRIKE 5
#define UMBRAL_BAD_REPLY_STRIKE 10
#define UMBRAL_BAD_COMM_STRIKE 10
#define UMBRAL_FOTORESISTENCIA 150
#define UMBRAL_PERSISTENCIA_FOTORESISTENCIA 3
#define UMBRAL_SABOTAJE 30.0
#define UMBRAL_SENSOR_INDIVIDUAL 99.0
#define UMBRAL_SENSOR_TOTAL 160.0
#define PORCENTAJE_SALTO_UNITARIO 15.0

#define FRACCION_SALTO 3

#include "Arduino.h"
#include "Macros.h"
#include "EventosJson.h"
#include "Registro.h"


extern Registro registro;
extern EventosJson eventosJson;
extern const char* literalesZonas[2][MAX_DATOS_SUB_TRAMA];
class Terminal {

private:
	byte TERMINAL_ID;
	char TERMINAL_NAME[5];
	byte NUM_SENSORES;
	byte NUM_LINEAS_CTL;
	byte NUM_FOTO_SENSOR;

	unsigned int NO_REPLY_STRIKE;
	unsigned int BAD_REPLY_STRIKE;
	unsigned int BAD_COMM_STRIKE;

	double porcentajeDeteccion = 0.0;

	int DATOS_FOTOSENSOR;
	byte datosControlLineas[MAX_DATOS_CTL_LINEA];

	int dimensionSamplesMapeo[MAX_DATOS_SUB_TRAMA]; //Define los umbrales por sensor

	int sampleSensoresCont[MAX_DATOS_SUB_TRAMA];
	int sampleSensoresPhantom[MAX_DATOS_SUB_TRAMA];
	unsigned long maxEjecucion = 0;


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
	void EvaluarSensorPhantom(Lista* lista);
	void recalcularNumDeteciones(Lista* lista);
	void purgarNodosViejos(Lista* lista);
	void purgarLista();
	void limpiarResultadoPhantom();
	byte nodosRevisados = 0;
	byte persistenciaFotoresistencia = 0;
	String generarCabecera(int numElementos);

	SAAS_PARAMETROS_SALTO saasParametrosSalto;


public:
	Terminal(byte id,
			char* nombreTerminal,
			byte numFotoSensor = 1,
			byte numLineasCtl = 2,
			byte numSensores = 8,
			int* dimensionSamplesMapeo = nullptr
	);

	InterpretacionTerminal interpretacion;
	RespuestaTerminal respuesta;

	byte getTerminalId() const;
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

	RespuestaTerminal evaluarDatosTerminal();
	void evaluarPhantomTerminal();
	void controlNodosEnMemoria();
	void limpiarDatosTerminal();

	String generarInformeDatos();

	int getSampleSensores(int numSensor);

	String  serializarListaJson();
	void deserializarListJson(String listaSerializada);


};

#endif /* MUSIC_SOURCE_TERMINAL_H_ */

