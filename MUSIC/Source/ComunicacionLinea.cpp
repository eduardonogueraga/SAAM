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
 *      SAAM -> COCHERA
 *      ###I#CH#MA#0#N#N#N#N#F###
 *
 *      TERMINAL -> SAAM
 *      ###I#MA#CH#1#N#1;0#63#0;0;1;1;1;1;1;1#F###
 *
 */

#include "ComunicacionLinea.h"

ComunicacionLinea::ComunicacionLinea() { // @suppress("Class members should be properly initialized")
	terminalComposer = LLAMAR_TERMINAL;
	lecturaLinea = SILENCIO;


	this->escucharRed = 1;
	this->numeroReintentosTerminal = 0;
	this->numeroReintentosMaster = 0;
}


byte ComunicacionLinea::getEscucharRed() const {
	return escucharRed;
}

void ComunicacionLinea::setEscucharRed(byte escucharRed) {
	this->escucharRed = escucharRed;
}

void ComunicacionLinea::mantenerComunicacion(){
	if(!configSystem.ESCUCHAR_LINEA || !ACCESO_LISTAS)
		return;


/*
	if(!escucharLinea(*T_LIST[0])){
		return;
	}
*/

	//Se interrogan a los terminales conectados
/*
	if(this->flagSalidaComposer == 0){ //@TEST
		Serial.println("TEST RS");
	}else {
		return;
	}
*/
	for (int i = 1; i < N_TERMINALES_LINEA; i++) {
		//El terminal core no se interroga comienza desde id 1
		while (!this->flagSalidaComposer) {
			this->interrogarTerminal(*T_LIST[i]);
			//vTaskDelay(100);
		}

		this->flagSalidaComposer = 0;
	}

	//this->flagSalidaComposer = 1; //@TEST
	//delay(500);

}


LecturasLinea ComunicacionLinea::escucharLinea(Terminal &terminal) {

	lecturaLinea = SILENCIO;

	if(gotoUart==1){ //Control serie
		gotoUart=0;
		const char datosFake[] = "###I#MA#CH#1#N#1;0#63#0;0;1;1;1;1;1;1#F###";
		strncpy(tramaRecibida, datosFake, sizeof(tramaRecibida) - 1);
		tramaRecibida[sizeof(tramaRecibida) - 1] = '\0';
		goto uartData;
	}


	if (UART_RS.available() > 0) {
		//delay(5); //Espera al paquete

		limpiarBuffer(tramaRecibida);

		byteCount = UART_RS.readBytesUntil('\n', tramaRecibida, sizeof(tramaRecibida) - 1); //read in data to buffer
		tramaRecibida[byteCount] = NULL;	//put an end character on the data

		uartData:

		Serial.println(tramaRecibida);

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

			//Datos basicos
			for (byte i = 0; i < MAX_DATOS_TRAMA; i++) {
				if(i!=0){
					token =  strtok(NULL, DELIMITADOR);
				}
				extraerDatosToken(i);
			}


			Serial.println("Trama principal:");
			for (byte i = 0; i < MAX_DATOS_TRAMA; i++) {
				Serial.print(i);
				Serial.print(": ");
				Serial.println(datosStrings[i]);
			}


			if (strcmp(datosStrings[AUTOR], terminal.getTerminalName()) == 0) {
				//EL REMITENTE ES EL CORRECTO, PROSIGUE LA EXTRACCION DE LOS DATOS
				  if (atoi(datosStrings[METODO]) == MTH_REPLY){

					//EL METODO ES DE RESPUESTA Y CONTIENE INFORMACION
					//SUBTRAMA SENSORES
					token =  strtok(NULL, DELIMITADOR);

					if ((token != NULL && strlen(token) > 1) && strlen(token) < sizeof(subTramaSensores)) {
						strncpy(subTramaSensores, token, sizeof(subTramaSensores) - 1);
						subTramaSensores[sizeof(subTramaSensores) - 1] = '\0';
					}
					else {
						Serial.print(F("ERROR AL INSERTAR SENSORES"));
					}


					Serial.println("Subtrama sensores:");
					extraerDatosSensores(subTramaSensores,valorSensores, terminal.getNumSensores());
					Serial.println("Subtrama estado del servicio:");
					extraerDatosSensores(datosStrings[ESTADO_SERVICIO],valorControlLineas, terminal.getNumLineasCtl());

					valorFotoSensor =  atoi(datosStrings[FOTOSENSOR]);

				}else {
					//Si es otro metodo sumo el resto de campos no tratados
					dataCount = dataCount + terminal.getNumSensores() + terminal.getNumLineasCtl();
				}

				Serial.println("Num datos recolectados:");
				Serial.println(dataCount);

				if (dataCount == (MAX_DATOS_TRAMA + terminal.getNumSensores() + terminal.getNumLineasCtl())){
					Serial.println(F("OK MAN"));

					terminal.setDatosFotosensor(valorFotoSensor); //@TEST ONLY
					terminal.guardarDatosTerminal(valorSensores, valorControlLineas); //@TEST ONLY

					lecturaLinea = TRAMA_OK;
				} else {
					Serial.println(F("FALTAN DATOS TRAMA (INCONSISENTE)"));
					lecturaLinea = TRAMA_KO;
				}

				return lecturaLinea;
			}
			else{
				Serial.println(F("REMITENTE INCORRECTO"));
				lecturaLinea = TRAMA_KO;
				return lecturaLinea;
			}

		}else{
			Serial.println(F("TRAMA SIN INICIO O FIN (INCONSISENTE)"));
			lecturaLinea = TRAMA_KO;
			return lecturaLinea;
		}

	}

	return lecturaLinea;

}

void ComunicacionLinea::extraerDatosToken(byte posicion){

	if ((token != NULL && strlen(token) > 0) && strlen(token) < sizeof(datosStrings[posicion])) {
		strncpy(datosStrings[posicion], token, sizeof(datosStrings[posicion]));
		dataCount++;
	}
	else {
		Serial.print(F("ERROR AL INSERTAR EL DATO: "));
		Serial.println(posicion);

	}

}


void ComunicacionLinea::extraerDatosSensores(char* subTrama, byte* arrSalida, byte maxDatosSubTrama) {

	for (byte i = 0; i < maxDatosSubTrama; i++) {
		if (i == 0) {
			token = strtok(subTrama, SUB_DELIMITADOR);
		}
		else {
			token = strtok(NULL, SUB_DELIMITADOR);
		}

		if ((token != NULL && strlen(token) > 0) && strlen(token) < sizeof(arrSalida)) {
			arrSalida[i] = atoi(token);
			dataCount++;
		}
		else {
			Serial.print(F("ERROR AL INSERTAR SENSOR: "));
			Serial.print(i);
		}
	}

	//Imprime datos TEST
	/*
	for (byte i = 0; i < maxDatosSubTrama; i++) {
		Serial.print(i);
		Serial.print(": ");
		Serial.println(arrSalida[i]);
	}
	*/

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
   Serial.println(tramaEnviada);
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
void ComunicacionLinea::constructorTramaDatos(Terminal &terminal, byte metodo){

	limpiarBuffer(tramaEnviada); //LIMPIA LA TRAMA ANTERIOR

	strncat(tramaEnviada, BLINDAJE_SERIE, sizeof(tramaEnviada));
	strncat(tramaEnviada, INICIO_TRAMA, sizeof(tramaEnviada));
	strncat(tramaEnviada, terminal.getTerminalName(), sizeof(tramaEnviada)); 		//DESTINATARIO
	strncat(tramaEnviada, DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, TERMINAL_NAME, sizeof(tramaEnviada));
	strncat(tramaEnviada, DELIMITADOR, sizeof(tramaEnviada));
	this->procesarMetodo(metodo);
	strncat(tramaEnviada, DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, CAMPO_NULO, sizeof(tramaEnviada));
	strncat(tramaEnviada, DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, CAMPO_NULO, sizeof(tramaEnviada));
	strncat(tramaEnviada, DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, CAMPO_NULO, sizeof(tramaEnviada));
	strncat(tramaEnviada, DELIMITADOR, sizeof(tramaEnviada));
	strncat(tramaEnviada, CAMPO_NULO, sizeof(tramaEnviada));
	strncat(tramaEnviada, FIN_TRAMA, sizeof(tramaEnviada));
	strncat(tramaEnviada, BLINDAJE_SERIE, sizeof(tramaEnviada));

	 sprintf(tramaEnviada, "%s\n", tramaEnviada);

}

void ComunicacionLinea::procesarMetodo(byte metodo){

	char metodoId[2];
	itoa(metodo, metodoId, 2);

	strncat(tramaEnviada, metodoId, sizeof(tramaEnviada));

	/*
	switch(metodo){

	case MTH_DATA:
		strncat(tramaEnviada, "DATA", sizeof(tramaEnviada));
		break;

	case MTH_RETRY:
		strncat(tramaEnviada, "RETRY", sizeof(tramaEnviada));
		break;

	case MTH_RESET:
		strncat(tramaEnviada, "RESET_OK", sizeof(tramaEnviada));
		break;

	}*/
}

void ComunicacionLinea::writeChar(char *TEXTO_ENVIO) {
	for (int i = 0; i < strlen(TEXTO_ENVIO); i++){
		UART_RS.write(TEXTO_ENVIO[i]);
		delay(5);
	}
}

void ComunicacionLinea::testUart(){
	gotoUart = 1;
}


void ComunicacionLinea::interrogarTerminal(Terminal &terminal){

	LecturasLinea lectura;

	switch (terminalComposer) {
	case LLAMAR_TERMINAL:
		digitalWrite(RS_CTL, HIGH);  //Enable max485 transmission

		this->constructorTramaDatos(terminal, MTH_DATA);
		//this->enviarTrazaDatos();

		  this->writeChar(tramaEnviada);
		  Serial.println(tramaEnviada);

		//Define el tiempo de espera
		tiempoEspera = millis() + TIEMPO_ESPERA_MASTER;
		terminalComposer = ESCUCHAR_LINEA;

		 Serial.println("RS: ESCUCHAR LINEA"); //@TEST
		 //this->testUart(); //@TEST
		break;

	case ESCUCHAR_LINEA:

		digitalWrite(RS_CTL,LOW);    //Disable max485 transmission mode

		//vTaskDelay(500);
		//this->testUart(); //@TEST

		//vTaskDelay(500);
		if (millis() < tiempoEspera) {


		    lectura = escucharLinea(terminal);

			if(lectura != SILENCIO){

				Serial.println("RS: SE RECIBE CONTENIDO"); //@TEST

				if (lectura == TRAMA_OK) {
					//Leer el metodo para identificar lo que nos ha trasmitido el terminal
					Serial.println("RS: TRAMA OK"); //@TEST

					strncpy(metodo, datosStrings[METODO], sizeof(metodo));

					if (atoi(datosStrings[METODO]) == MTH_REPLY) {

						//El terminal devuelve la informacion OK

						Serial.println("RS: METODO DATA"); //@TEST

						terminal.setDatosFotosensor(valorFotoSensor);
						terminal.guardarDatosTerminal(valorSensores, valorControlLineas);

						terminal.limpiarStrikes();
						this->numeroReintentosTerminal = 0;
						this->numeroReintentosMaster = 0;

						terminalComposer = LLAMAR_TERMINAL;
						this->flagSalidaComposer = 1;

					}else if(atoi(datosStrings[METODO]) == MTH_RETRY){
						//El terminal destino no entendio nuestra peticion y solicita reintento
						Serial.println("RS: METODO REINTENTO"); //@TEST

						terminal.addBadCommStrike();
						this->numeroReintentosMaster++;
						terminalComposer = REINTENTAR;


					} else	{
						//Metodo  reconocido solicitamos reintento
						Serial.println("RS: N-A METODO"); //@TEST

						terminal.addBadReplyStrike();
						this->numeroReintentosTerminal++;
						terminalComposer = SOLICITAR_REINTENTO;
					}


				} else {
					//Trama incorrecta solicitamos reintento
					Serial.println("RS: TRAMA CORRUPTA"); //@TEST
					terminal.addBadReplyStrike();
					this->numeroReintentosTerminal++;
					terminalComposer = SOLICITAR_REINTENTO;
				}

			}

		} else {
			// No se obtuvo respuesta marcamos strike al terminal y saltamos al siguente
			Serial.println("RS: SIN RESPUESTA"); //@TEST
			terminal.addNoReplyStrike();
			terminalComposer = LLAMAR_TERMINAL;

			this->flagSalidaComposer = 1;
		}
		break;

	case SOLICITAR_REINTENTO:
		// Generar la traza correspondiente y entender la respuesta
		Serial.println("RS: SOLICTAMOS REINTENTO"); //@TEST

		    Serial.print("RS: NUM REINTENTOS: "); //@TEST
			Serial.println(numeroReintentosTerminal); //@TEST

		if(numeroReintentosTerminal < 2){
			digitalWrite(RS_CTL, HIGH);  //Enable max485 transmission

			this->constructorTramaDatos(terminal, MTH_RETRY);
			//this->enviarTrazaDatos();


			  this->writeChar(tramaEnviada);
			  Serial.println(tramaEnviada);

			//Define el tiempo de espera
			tiempoEspera = millis() + TIEMPO_ESPERA_MASTER;

			//this->testUart(); //@TEST
			terminalComposer = ESCUCHAR_LINEA;
		}else {
			//Demasiados reintentos iteramos al siguente terminal
			Serial.println("RS: DEMASIADOS REINTENTOS"); //@TEST
			terminalComposer = LLAMAR_TERMINAL;
			this->flagSalidaComposer = 1;
		}

		break;


	case REINTENTAR:
		Serial.println("RS: REINTENTO MASTER"); //@TEST
		terminalComposer = LLAMAR_TERMINAL;
		Serial.print("RS: NUM REINTENTOS: "); //@TEST
		Serial.println(numeroReintentosMaster); //@TEST

		if(numeroReintentosMaster >= MAX_REINTENTOS_MASTER){
			Serial.println("RS: DEMASIADOS REINTENTOS 2"); //@TEST
			this->flagSalidaComposer = 1;
		}

		break;
	}

}

