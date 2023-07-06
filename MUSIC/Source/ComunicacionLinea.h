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

extern HardwareSerial UART_RS;

class ComunicacionLinea {



private:
  byte dataCount = 0; //NUMERO DATOS RECOPILADOS
  char *token;
  char tramaRecibida[200] = "";
  char tramaEnviada[200] = "";
  char subTramaSensores[30] = "";
  const char INICIO_TRAMA[10] = "###INIT#";
  const char FIN_TRAMA[10] = "#END###";
  const char DELIMITADOR[2] = "#";
  const char SUB_DELIMITADOR[2] = ";";
  const char TERMINAL_NAME[7] = "MASTER";
  char datosStrings[MAX_DATOS_TRAMA][10];
  byte valorSensores[MAX_DATOS_SUB_TRAMA]; //Numero de sensores por terminal
  byte valorControlLineas[MAX_DATOS_CTL_LINEA]; //Numero de lineas de control por terminal
  size_t byteCount;



  bool escucharLinea();
  void enviarTrazaDatos();
  void extraerDatosToken(byte num = 0);
  void extraerDatosSensores(char* subTrama, byte* arrSalida, byte maxDatosSubTrama);
  void borrarDatosStrings();
  void limpiarBuffer(char*);
  void constructorTramaDatos(byte metodo = 0);
  void procesarMetodo(byte metodo);
  void writeChar(char *TEXTO_ENVIO);

  byte gotoUart= 0;

public:
	ComunicacionLinea();
	void mantenerComunicacion();
	void testUart();

};

#endif /* SOURCE_COMUNICACIONLINEA_H_ */
