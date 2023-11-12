/*
 * HttpRequest.h
 *
 *  Created on: 2 sept. 2023
 *      Author: Eduardo
 *
 *      Funciones encargadas de gestionar las peticiones al servidor
 */

#ifndef SOURCE_ALARMAUTIL_HTTPREQUEST_H_
#define SOURCE_ALARMAUTIL_HTTPREQUEST_H_

String eliminarCaracteresRaros(String cadena) {
    String cadenaLimpia = "";
    for (int i = 0; i < cadena.length(); i++) {
        char caracter = cadena.charAt(i);
        if (isPrintable(caracter)) {
            cadenaLimpia += caracter;
        }
    }
    return cadenaLimpia;
}

String cifrarCadena(const String& t) {

	byte iv[N_BLOCK];
	int msgLen = t.length();
	char encrypted[2 * msgLen] = {0};

	//Preparar clave en byte
	byte aes_key[strlen(AES_KEY)];

	for (int i = 0; i < strlen(AES_KEY); i++) {
		aes_key[i] = (byte)AES_KEY[i];
		iv[i] = AES_KEY_IV[i];
	}

	aesLib.encrypt64((const byte*)t.c_str(), msgLen, encrypted, aes_key, sizeof(aes_key), iv);
	return String(encrypted);
}


String descifrarCadena(const String& inputString) {

	byte iv[N_BLOCK];
	int msgLen = inputString.length();
	char decrypted[msgLen + 1] = {0}; // +1 para el caracter nulo al final
	const char* ciphertext = inputString.c_str(); // Obtener puntero a la representación de caracteres del String

	// Crear un bufer temporal para los datos cifrados
	char tempCiphertext[msgLen];
	strcpy(tempCiphertext, ciphertext);

	//Preparar clave en byte
	byte aes_key[strlen(AES_KEY)];

	for (int i = 0; i < strlen(AES_KEY); i++) {
		aes_key[i] = (byte)AES_KEY[i];
		iv[i] = AES_KEY_IV[i];
	}

	aesLib.decrypt64(tempCiphertext, msgLen, (byte*)decrypted, aes_key, sizeof(aes_key), iv);
	return String(decrypted);
}

	void escucharGSM(){

		if (UART_GSM.available() > 0) { // Verificamos si hay datos disponibles para leer

			char tramaRecibida[200] = "";
			size_t byteCount = UART_GSM.readBytesUntil('\n', tramaRecibida, sizeof(tramaRecibida) - 1); //read in data to buffer
			tramaRecibida[byteCount] = NULL;	//put an end character on the data


			Serial.print("UART@GSM-> ");
			Serial.println(tramaRecibida);
			UART_GSM.flush();

		}

	}

	ProveedorEstado coberturaRed() {

		static ProveedorEstado respuesta;
		static unsigned long tiempoAnterior = 0; // Variable estática para almacenar el tiempo de la última ejecución

		unsigned long tiempoActual = millis(); // Obtener el tiempo actual desde que se encendió Arduino

		// Verificar si han pasado al menos 10 segundos desde la última ejecución
		if (tiempoActual - tiempoAnterior >= 4000) { // 10000 milisegundos = 10 segundos
			tiempoAnterior = tiempoActual; // Actualizar el tiempo de la última ejecución

			respuesta.intensidadSignal = modem.getSignalQuality();
			respuesta.proveedor = modem.getOperator();

			if(respuesta.proveedor.isEmpty())
				respuesta.proveedor = "Sin servicio";

			return respuesta;
		}

		// Si no han pasado 10 segundos, no ejecutar la función y devolver un valor inválido (por ejemplo, -1)
		return respuesta;
	}

	void refrescarModuloGSM(){
		setMargenTiempo(tiempoRefrescoGSM, 5000);
	}

	void comprobarConexionGSM(unsigned long timeOut){
		Serial.print("Waiting for network...");
		pantalla.lcdLoadView(&pantalla, &Pantalla::sysConexionGSM);
		if (!modem.waitForNetwork(timeOut, true)) {
			Serial.println(" fail");
			registro.registrarLogSistema("Error! Sin red movil");

			pantallaDeError(F("  SYSTM ERROR!   SIN RED MOVIL  "));
			return;
		}
		Serial.println(" success");
		if (modem.isNetworkConnected()) { Serial.println("Network connected"); }


		String operatorName = modem.getOperator();
		Serial.print("Proveedor de servicios: ");
		Serial.println(operatorName);

		int csq = modem.getSignalQuality();
		Serial.println("Signal quality: " + String(csq));


		char descripcion[190];
		snprintf(descripcion, sizeof(descripcion), "Operador: %s, Calidad de red: (%d)", operatorName.c_str(), csq);
		registro.registrarLogSistema(descripcion);


		int voltage = modem.getBattVoltage();
        Serial.print("Voltaje bateria: ");
		Serial.println(voltage);

		pantallaDeError(fixedLengthString(operatorName, 16)+"Calidad red:"+(csq));

	}

	bool establecerConexionGPRS(){
		// Intenta conectar al servicio de red GPRS
		Serial.println("Conectando a la red GPRS...");

		if(xPortGetCoreID() == 1)
		pantalla.lcdLoadView(&pantalla, &Pantalla::sysConexionGprs);

		if (!modem.gprsConnect(apn, gsmUser, gsmPass)) {
			Serial.println(" Error al conectar a la red GPRS.");
			if(xPortGetCoreID() == 1)
			pantalla.lcdLoadView(&pantalla, &Pantalla::sysConexionGprsFail);

			return false;
		}
		Serial.println(" Conexion a la red GPRS establecida.");
		if(xPortGetCoreID() == 1)
		pantalla.lcdLoadView(&pantalla, &Pantalla::sysConexionGprsOk);

		return true;
	}

	void cerrarConexionGPRS(){
		modem.gprsDisconnect();
		Serial.println(F("GPRS disconnected"));
	}

	RespuestaHttp realizarPeticionHttp(const char* metodo, const char* resource, byte auth = 1, const char* jsonData = nullptr){

		RespuestaHttp respuesta;

		if(establecerConexionGPRS()){
			int estadoHttp = 0;
			String respuestaHttp;
			respuestaHttp += "Inicio peticion "+ fecha.imprimeFecha(1) +"\n";

			String ipAdd = modem.getLocalIP();
			Serial.print("Ip asignada: ");
			Serial.println(ipAdd);

			respuestaHttp += "Direccion asignada GPRS "+ ipAdd +"\n";

			//vTaskDelay(1000);

			//Formular peticion HTTP
			http.connectionKeepAlive();  // Currently, this is needed for HTTPS
			http.beginRequest();

			if (strcmp(metodo, "GET") == 0) {
				estadoHttp = http.get(resource);
			} else if (strcmp(metodo, "POST") == 0) {
				if (jsonData) {
					estadoHttp= http.post(resource);
					//http.sendHeader("Content-Type", "application/json");
					http.sendHeader("Content-Type", "text/plain");
					http.sendHeader("Content-Length", strlen(jsonData));

				} else {
					estadoHttp = http.post(resource);
				}
			}


			if(auth){
				Serial.println("Autorizando peticion...");
				String SAAS_TOKEN = leerCadenaEE("SAAS_TOKEN");
				Serial.println(SAAS_TOKEN);
				http.sendHeader("Authorization", String("Bearer ") + SAAS_TOKEN);
			}

			//Adjuntamos el json en el body
			http.beginBody();

			if (jsonData){
				int len = strlen(jsonData);

				Serial.println("Buffer a enviar " + String(len));
				respuestaHttp += "Buffer a enviar "+ String(len) +"\n";

				uint32_t j = 0;
				uint32_t shard = 512;
				for (int32_t i = len; i > 0;) {
					if (i >= shard) {
						http.write((const unsigned char *)(jsonData + shard * j), shard);
						i -= shard;
						j++;
					} else {
						http.write((const unsigned char *)(jsonData + shard * j), i);
						break;
					}
				}

			}

			http.endRequest();


			//Gestionar respuesta HTTP
			respuesta.codigo = http.responseStatusCode();
			Serial.print(F("Response status code: "));
			Serial.println(respuesta.codigo);

			if (estadoHttp != 0 || respuesta.codigo <= 0) {
				Serial.println(F("failed to connect"));

				if(xPortGetCoreID() == 1)
				pantallaDeError(F("  SYSTM ERROR!  PETICION HTTP KO"));

				respuestaHttp += "Fallo en la peticion HTTP error API: "
						+ String(estadoHttp)
						+ " error HTTP:"
						+ String(respuesta.codigo) +"\n";
			}else {
				respuestaHttp += "Codigo respuesta servidor: " + String(respuesta.codigo) + "\n";

				byte linesCount = 0;
				Serial.println(F("Response Headers:"));
				while (http.headerAvailable() && linesCount < 15) {
					String headerName  = http.readHeaderName();
					String headerValue = http.readHeaderValue();
					Serial.println("    " + headerName + " : " + headerValue);
					respuestaHttp += "    " + headerName + " : " + headerValue + "\n";

				}

				int length = http.contentLength();

				if (length >= 0) {
					Serial.print(F("Content length is: "));
					Serial.println(length);
					respuestaHttp += "Content length :"+ (String)length + "\n";
				}

				if (http.isResponseChunked()) {
					Serial.println(F("The response is chunked"));
					respuestaHttp += "The response is chunked\n";
				}

				String body = http.responseBody();
				Serial.println(F("Response:"));
				Serial.println(body);

				//Comprobamos si la respuesta es plana o json
				DynamicJsonDocument respuetaJson(100);

				if (body.charAt(0) == '{' && body.charAt(body.length() - 1) == '}') {

					Serial.println("Respuesta JSON");
					DeserializationError error = deserializeJson(respuetaJson, body);

					if(!error){
						String msgJson = respuetaJson["msg"];
						String errorJson  = respuetaJson["error"];

						respuesta.respuesta = (respuesta.codigo == 200) ? msgJson : errorJson;
						respuestaHttp += "Respuesta servidor: " + String(respuesta.respuesta) + "\n";

					}else {
						Serial.println("Error deserializando JSON");
						respuestaHttp += "Respuesta servidor: Error saa durante la extraccion JSON\n";
					}

				} else {
					Serial.println("Respuesta NO JSON");
					respuesta.respuesta = body;
					respuestaHttp += "Respuesta servidor: " + String(respuesta.respuesta) + "\n";

				}

				Serial.print(F("Body length is: "));
				Serial.println(body.length());
				respuestaHttp += "Body length is: " + String(body.length()) + "\n";

				http.stop(); // Shutdown
				Serial.println(F("Server disconnected"));
			}

			respuestaHttp += "Fin peticion "+ fecha.imprimeFecha(1) +"\n";

			RegistroLogTarea reg;
			TickType_t espera = pdMS_TO_TICKS(10);
			respuestaHttp.toCharArray(reg.log, sizeof(reg.log));
			reg.tipoLog = 1; //http

			xQueueSend(colaRegistros, &reg, espera);


			cerrarConexionGPRS();

			vTaskDelay(300); //Damos algo de tipo a la cola de registros para que no se pisen
		}
		return respuesta;
	}


	int getIdPaqueteSaas(){
		RespuestaHttp respuesta;
	    respuesta = realizarPeticionHttp("GET", getUltimoPaquete);

	    Serial.print("Codigo: ");
	    Serial.println(respuesta.codigo);
	    Serial.print("Contenido: ");
	    Serial.println(respuesta.respuesta);

	    respuesta.respuesta = descifrarCadena(respuesta.respuesta);
	    respuesta.respuesta.trim();
	    respuesta.respuesta = eliminarCaracteresRaros(respuesta.respuesta);
	    Serial.print("Contenido descifrado: ");
	    Serial.println(respuesta.respuesta);

	    if(respuesta.codigo == 200){
	    	int id = respuesta.respuesta.toInt();
	    	guardarFlagEE("PACKAGE_ID", id);
	    }

	    return respuesta.codigo;
	}

	int generarTokenSaas(){
		RespuestaHttp respuesta;
		respuesta = realizarPeticionHttp("POST", postTokenSanctum, 0);

	    Serial.print("Codigo: ");
	    Serial.println(respuesta.codigo);
	    Serial.print("Contenido: ");
	    Serial.println(respuesta.respuesta);

	    respuesta.respuesta = descifrarCadena(respuesta.respuesta);
	    respuesta.respuesta.trim();
	    respuesta.respuesta = eliminarCaracteresRaros(respuesta.respuesta);

	    Serial.print("Contenido descifrado: ");
	    Serial.println(respuesta.respuesta);

		if(respuesta.codigo == 200){
			guardarCadenaEE("SAAS_TOKEN", &respuesta.respuesta);
		}

		return respuesta.codigo;
	}



	RespuestaHttp postDatosSaas(String* modeloJson, SAAS_TIPO_HTTP_REQUEST tipoDatos){
		RespuestaHttp respuesta;

		*modeloJson = cifrarCadena(*modeloJson);

		if(tipoDatos == PAQUETE){
			 Serial.println("Envio de paquete");
			respuesta = realizarPeticionHttp("POST", postEventosJson, 1, modeloJson->c_str());
		}else if(tipoDatos == NOTIFICACION){
			Serial.println("Envio de notificacion");
			respuesta = realizarPeticionHttp("POST", postNotificacionJson, 1, modeloJson->c_str());
		}else {
			Serial.print("Error tipo de peticion http no reconocida");
		}

	    Serial.print("Codigo: ");
	    Serial.println(respuesta.codigo);
	    Serial.print("Contenido: ");
	    Serial.println(respuesta.respuesta);

		return respuesta;
	}

	uint8_t sendATcommand(const char* ATcommand, const char* expected_answer, unsigned int timeout) {

	  uint8_t x = 0,  answer = 0;
	  char response[100];
	  unsigned long previous;

	  memset(response, '\0', 100);    // Initialize the string

	  delay(100);

	  while ( UART_GSM.available() > 0) UART_GSM.read();   // Clean the input buffer

	  UART_GSM.println(ATcommand);    // Send the AT command


	  x = 0;
	  previous = millis();

	  // this loop waits for the answer
	  do {
	    if (UART_GSM.available() != 0) {
	      // if there are data in the UART input buffer, reads it and checks for the asnwer
	      response[x] = UART_GSM.read();
	                  Serial.print(response[x]);
	      x++;
	      // check if the desired answer  is in the response of the module
	      if (strstr(response, expected_answer) != NULL)
	      {
	        answer = 1;
	      }
	    }
	    // Waits for the asnwer with time out
	  } while ((answer == 0) && ((millis() - previous) < timeout));

	  //    Serial.print("\n");

	  return answer;
	}

	void ConfigureFTP(const char* FTPServer, const char* FTPUserName, const char* FTPPassWord) {
	  char aux_str[50];

	  int result;
	  int response;

	  result = sendATcommand("AT+CFTPPORT=7512", "OK", 2000);
	  Serial.print("AT+CFTPPORT=7512: ");
	  Serial.println(result);

	  result = sendATcommand("AT+CFTPMODE=1", "OK", 2000);
	  Serial.print("AT+CFTPMODE=1: ");
	  Serial.println(result);

	  result = sendATcommand("AT+CFTPTYPE=A", "OK", 2000);
	  Serial.print("AT+CFTPTYPE=A: ");
	  Serial.println(result);


	  sprintf(aux_str, "AT+CFTPSERV=\"%s\"", FTPServer);
	  response = sendATcommand(aux_str, "OK", 2000);
	  Serial.print(aux_str);
	  Serial.print(": ");
	  Serial.println(response);

	  sprintf(aux_str, "AT+CFTPUN=\"%s\"", FTPUserName);
	  response = sendATcommand(aux_str, "OK", 2000);
	  Serial.print(aux_str);
	  Serial.print(": ");
	  Serial.println(response);

	  sprintf(aux_str, "AT+CFTPPW=\"%s\"", FTPPassWord);
	  response = sendATcommand(aux_str, "OK", 2000);
	  Serial.print(aux_str);
	  Serial.print(": ");
	  Serial.println(response);


	  char FTPConnectionCommand[150];
	   sprintf(FTPConnectionCommand, "AT+CFTPSLOGIN=\"%s\",%i,\"%s\", \"%s\",0", FTPServer, 7512, FTPUserName, FTPPassWord);
	   if(!sendATcommand("AT+CFTPSSTART","OK",2000)){ Serial.println("ok1");}
	   delay(200);
	   if(!sendATcommand(FTPConnectionCommand,"OK",2000)){Serial.println("ok2");}
	   delay(2500);



	}



	void DownloadFromFTP(const char* FileName) {
	  char aux_str[50];



	  Serial.print("Download file from FTP...\n");
	  sprintf(aux_str, "AT+CFTPGETFILE=\"%s\",0", FileName);
	  int result = sendATcommand(aux_str, "OK", 2000);

	   if (result == 1) {
	     Serial.println("Descarga exitosa.");
	   } else {
	     Serial.println("Error en la descarga.");
	   }

	}

	void testEnvioFtp(){

		//uint8_t answer = 0;
		//answer = sendATcommand("AT", "OK", 2000);
		//Serial.println(answer);

	  // Llama a la función ConfigureFTP con los detalles del servidor FTP
	  const char* servidorFTP = "";
	  const char* userFtp = "";
	  const char* passFtp = "";
	  ConfigureFTP(servidorFTP, userFtp, passFtp);

	  //const char* nombreArchivo = "test.txt";
	  //DownloadFromFTP(nombreArchivo);


	}

	void pruebaCifrado(){

		//String inputString = R"()";
		String inputString = "eIEHqE/vOVLb7FO9VBDD/6ABzwzlo+OPMP9HhvMs7YQTIib9fxpCgnIxtAwt9uYT";

		// Encrypt Data
		String encrypted = cifrarCadena(inputString);

		Serial.print("Base64 encoded Ciphertext: ");
		Serial.println(encrypted);

		 encrypted = "eIEHqE/vOVLb7FO9VBDD/6ABzwzlo+OPMP9HhvMs7YQTIib9fxpCgnIxtAwt9uYT";

		// Decrypt Data
		String decrypted = descifrarCadena(encrypted);

		Serial.print("Base64-decoded Cleartext: ");
		Serial.println(decrypted);

	}


#endif /* SOURCE_ALARMAUTIL_HTTPREQUEST_H_ */
