/*
 * ComunicacionLinea.h
 *
 *  Created on: 4 jul. 2023
 *      Author: Eduardo
 */

#ifndef SOURCE_COMUNICACIONLINEA_H_
#define SOURCE_COMUNICACIONLINEA_H_

#include "Arduino.h"
#include "Macros.h"
#include <HardwareSerial.h>
#include "Terminal.h"
#include <Adafruit_MCP23X17.h>

#define TIEMPO_ESPERA_MASTER 2500

extern HardwareSerial UART_RS;
extern Terminal* T_LIST[];
extern Adafruit_MCP23X17 mcp;
extern byte ACCESO_LISTAS;

class ComunicacionLinea {



private:

	byte escucharRed;

	char metodo[10];
	char metodoDatos[10] = "DATA";
	char metodoReset[10] = "RESET";
	char metodoRespuesta[10] = "REPLY";
	char metodoReintento[10] = "RETRY";

	byte dataCount = 0; //NUMERO DATOS RECOPILADOS
	char *token;
	char tramaRecibida[200] = "";
	char tramaEnviada[200] = "";
	char subTramaSensores[30] = "";
	const char INICIO_TRAMA[5] = "I#";
	const char FIN_TRAMA[5] = "#F";
	const char BLINDAJE_SERIE[5] = "###";
	const char DELIMITADOR[2] = "#";
	const char CAMPO_NULO[2] = "N";
	const char SUB_DELIMITADOR[2] = ";";
	const char TERMINAL_NAME[5] = "MA";
	char datosStrings[MAX_DATOS_TRAMA][10];
	byte valorSensores[MAX_DATOS_SUB_TRAMA]; //Numero de sensores por terminal
	byte valorControlLineas[MAX_DATOS_CTL_LINEA]; //Numero de lineas de control por terminal
	int	 valorFotoSensor;
	size_t byteCount;
	byte gotoUart= 0;
	unsigned long tiempoEspera;
	byte numeroReintentosTerminal;
	byte numeroReintentosMaster;
	byte flagSalidaComposer = 0;

	LecturasLinea lecturaLinea;
	EstadosTerminal terminalComposer;

	LecturasLinea escucharLinea(Terminal &terminal);
	void enviarTrazaDatos();
	void extraerDatosToken(byte num = 0);
	void extraerDatosSensores(char* subTrama, byte* arrSalida, byte maxDatosSubTrama);
	void borrarDatosStrings();
	void limpiarBuffer(char*);
	void constructorTramaDatos(Terminal &terminal, byte metodo = 0);
	void procesarMetodo(byte metodo);
	void writeChar(char *TEXTO_ENVIO);
	void interrogarTerminal(Terminal &terminal);

	void setEscucharRed(byte escucharRed);

public:
	ComunicacionLinea();
	void mantenerComunicacion();
	void testUart();

	byte getEscucharRed() const;

};

#endif /* SOURCE_COMUNICACIONLINEA_H_ */
