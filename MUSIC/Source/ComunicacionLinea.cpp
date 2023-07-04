/*
 * ComunicacionLinea.cpp
 *
 *  Created on: 4 jul. 2023
 *      Author: Eduardo
 *
 *      Esta clase se encarga de extraer los datos
 *     	de los terminales conectados a la linea por medio
 *     	del protocolo RS485
 *
 *
 *      Ejemplos trazas
 *
 *      SAAM -> TERMINAL
 *      ###INIT#PORCHE#MASTER#DATA#NONE#NONE#NONE#NONE#END###
 *
 *      TERMINAL -> SAAM
 *      ###INIT##MASTER#PORCHE#REPLY#NONE#L1:0;L2:0#19#PL1-1:0;PL1-2:0;PL1-3:0;PL1-4:0;PL2-1:0;PL2-2:0;PL2-3:0;PL2-4:0#END###
 *
 */

#include "ComunicacionLinea.h"

ComunicacionLinea::ComunicacionLinea() { // @suppress("Class members should be properly initialized")
	// TODO Auto-generated constructor stub
}

void ComunicacionLinea::mantenerComunicacion(){
	//SI LA TRAMA ES VALIDA LA PROCESA
	if(!escucharLinea()){
		return;
	}

	//String metodo = datosStrings[METODO];

	char metodo[20];
	char metodoDatos[20] = "DATA";
	char metodoReset[20] = "RESET";
	strncpy(metodo, datosStrings[METODO], sizeof(metodo));


	//DETERMINA LA RESPUESTA
	if (strcmp(metodo, metodoDatos) == 0) {
		this->constructorTramaDatos(MTH_DATA);
		this->enviarTrazaDatos();
	}else if(strcmp(metodo, metodoReset) == 0){
		this->constructorTramaDatos(MTH_RESET);
		this->enviarTrazaDatos();

		//resetear();
	}
	else	{
		this->constructorTramaDatos(MTH_RETRY);
		this->enviarTrazaDatos();
	}

}


bool ComunicacionLinea::escucharLinea() {

	if (UART_RS.available() > 0) {
		delay(5); //Espera al paquete

		limpiarBuffer(tramaRecibida);

		size_t byteCount = UART_RS.readBytesUntil('\n', tramaRecibida, sizeof(tramaRecibida) - 1); //read in data to buffer
		tramaRecibida[byteCount] = NULL;	//put an end character on the data
		//Serial.println(tramaRecibida);

		UART_RS.flush();
		char *inicioPuntero = strstr(tramaRecibida, INICIO_TRAMA);
		char *finalPuntero = strstr(tramaRecibida, FIN_TRAMA);

		dataCount = 0;

		if (inicioPuntero != NULL && finalPuntero != NULL) {

			byte finalTrama = finalPuntero - tramaRecibida; //CORTA TRAS EL FIN DE LA TRAMA
			tramaRecibida[finalTrama] = NULL;

			//Serial.println(tramaRecibida);

			int cursorDatos = (inicioPuntero - tramaRecibida) + strlen(INICIO_TRAMA); //POSICION TRAS EL INICIO DE LA TRAMA

			token =  strtok(&tramaRecibida[cursorDatos], DELIMITADOR);
			extraerDatosToken();


			if (strcmp(datosStrings[DESTINATARIO], TERMINAL_NAME) == 0) {
				//LLAMAN A ESTE DISPOSITIVO, PROSIGUE LA EXTRACCION DE LOS DATOS

				for (byte i = 1; i < MAX_DATOS_TRAMA; i++) {
					token =  strtok(NULL, DELIMITADOR);
					extraerDatosToken(i);
				}

				//Serial.println("DATOS RECUPERADOS: " + String(dataCount));

				if (dataCount == MAX_DATOS_TRAMA){

					/*
					Serial.println(" SALIDA:"); //TEST

					for (byte i = 0; i < MAX_DATOS_TRAMA; i++){
						Serial.println(datosStrings[i]);

					}
					*/

					return true;

				}else {
					Serial.println(F("FALTAN DATOS TRAMA (INCONSISENTE)"));
				}
			}
			else{
				Serial.println(F("TERMINAL NO RELACIONADO"));
			}

		}else{
			Serial.println(F("TRAMA SIN INICIO O FIN (INCONSISENTE)"));

		}

	}

	return false;

}

void ComunicacionLinea::extraerDatosToken(byte posicion){

	if ((token != NULL && strlen(token) > 1) && strlen(token) < sizeof(datosStrings[posicion])) {
		strncpy(datosStrings[posicion], token, sizeof(datosStrings[posicion]));
		dataCount++;
	}
	else {
		Serial.println(F("ERROR AL INSERTAR EL DATO: "));
		Serial.print(posicion);

	}

}

void ComunicacionLinea::borrarDatosStrings(){
	for(byte i = 0; i < MAX_DATOS_TRAMA; i++){
		for(byte j = 0; j < 10; j++){
			datosStrings[i][j] = NULL;
		}
	}
}


void ComunicacionLinea::limpiarBuffer(char *str){
	memset(str, 0, sizeof str);
}


void ComunicacionLinea::enviarTrazaDatos(){
   digitalWrite(RS_CTL, HIGH);  //Enable max485 transmission
   this->writeChar(tramaEnviada);
   digitalWrite(RS_CTL,LOW);    //Disable max485 transmission mode
}


/*
 *	Funcion constructorTramaDatos
 *
 *	METODOS -> MASTER (DATA, STATUS, RESET?)
 *	->ESCALVOS (REPLY (metedo accion ejecutada OK), RETRY (fallo al ejecutar KO))
 *
 *	###INIT#DESTINATARIO#REMITENTE#METODO#DETALLE-ERROR#L1:OK;L2:OK#FOTOSENSIBLE#SENSOR-NOMBRE:NUMERO#END###
 *
 *	###INIT#PORCHE#MASTER#DATA#NONE#NONE#NONE#NONE#END###
 */
void ComunicacionLinea::constructorTramaDatos(byte metodo){

/*
    sprintf(VALOR_FOTORESISTENCIA, "%d", sensor.getValorFotoResistencia());
	mapeoSensores = sensor.getMuestrasSensor();

	limpiarBuffer(tramaEnviada); //LIMPIA LA TRAMA ANTERIOR

	strncat(tramaEnviada, INICIO_TRAMA, sizeof(tramaEnviada));
	strncat(tramaEnviada, DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, "MASTER", sizeof(tramaEnviada)); 		//DESTINATARIO
	strncat(tramaEnviada, DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, TERMINAL_NAME, sizeof(tramaEnviada));
	strncat(tramaEnviada, DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, (metodo == MTH_RETRY)?"RETRY":"REPLY", sizeof(tramaEnviada));
	strncat(tramaEnviada, DELIMITADOR, sizeof(tramaEnviada));
	//strncat(tramaEnviada, (metodo == MTH_RETRY)?"MTH_FAIL":"NONE", sizeof(tramaEnviada));
	this->procesarMetodo(metodo);
	strncat(tramaEnviada, DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, "L1:", sizeof(tramaEnviada));
	strncat(tramaEnviada, (mapeoSensores[C01_LINEA])?"1":"0", sizeof(tramaEnviada));
	strncat(tramaEnviada, SUB_DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, "L2:", sizeof(tramaEnviada));
	strncat(tramaEnviada, (mapeoSensores[C02_LINEA])?"1":"0", sizeof(tramaEnviada));
	strncat(tramaEnviada, DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, VALOR_FOTORESISTENCIA, sizeof(tramaEnviada)); 		//FOTORESISTENCIA
	strncat(tramaEnviada, DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, "PL1-1:", sizeof(tramaEnviada));
	strncat(tramaEnviada, (mapeoSensores[L101_SENSOR])?"1":"0", sizeof(tramaEnviada));
	strncat(tramaEnviada, SUB_DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, "PL1-2:", sizeof(tramaEnviada));
	strncat(tramaEnviada, (mapeoSensores[L102_SENSOR])?"1":"0", sizeof(tramaEnviada));
	strncat(tramaEnviada, SUB_DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, "PL1-3:", sizeof(tramaEnviada));
	strncat(tramaEnviada, (mapeoSensores[L103_SENSOR])?"1":"0", sizeof(tramaEnviada));
	strncat(tramaEnviada, SUB_DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, "PL1-4:", sizeof(tramaEnviada));
	strncat(tramaEnviada, (mapeoSensores[L104_SENSOR])?"1":"0", sizeof(tramaEnviada));
	strncat(tramaEnviada, SUB_DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, "PL2-1:", sizeof(tramaEnviada));
	strncat(tramaEnviada, (mapeoSensores[L201_SENSOR])?"1":"0", sizeof(tramaEnviada));
	strncat(tramaEnviada, SUB_DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, "PL2-2:", sizeof(tramaEnviada));
	strncat(tramaEnviada, (mapeoSensores[L202_SENSOR])?"1":"0", sizeof(tramaEnviada));
	strncat(tramaEnviada, SUB_DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, "PL2-3:", sizeof(tramaEnviada));
	strncat(tramaEnviada, (mapeoSensores[L203_SENSOR])?"1":"0", sizeof(tramaEnviada));
	strncat(tramaEnviada, SUB_DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, "PL2-4:", sizeof(tramaEnviada));
	strncat(tramaEnviada, (mapeoSensores[L204_SENSOR])?"1":"0", sizeof(tramaEnviada));
	strncat(tramaEnviada, FIN_TRAMA, sizeof(tramaEnviada));
*/
}

void ComunicacionLinea::procesarMetodo(byte metodo){

	switch(metodo){

	case MTH_DATA:
		strncat(tramaEnviada, "NONE", sizeof(tramaEnviada));
		break;

	case MTH_RETRY:
		strncat(tramaEnviada, "MTH_FAIL", sizeof(tramaEnviada));
		break;

	case MTH_RESET:
		strncat(tramaEnviada, "RESET_OK", sizeof(tramaEnviada));
		break;

	}
}

void ComunicacionLinea::writeChar(char *TEXTO_ENVIO) {
	for (int i = 0; i < strlen(TEXTO_ENVIO); i++){
		UART_RS.write(TEXTO_ENVIO[i]);
		delay(5);
	}
}
