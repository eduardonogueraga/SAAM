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
		setMargenTiempo(tiempoRefrescoGSM, 1000);
		sleepModeGSM = GSM_REFRESH;
	}

	void comprobarConexionGSM(unsigned long timeOut){
		Serial.print("Waiting for network...");
		pantalla.lcdLoadView(&pantalla, &Pantalla::sysConexionGSM);
		if (!modem.waitForNetwork(timeOut, true)) {
			Serial.println(" fail");
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

			//vTaskDelay(1000);

			//Formular peticion HTTP
			//http.connectionKeepAlive();  // Currently, this is needed for HTTPS
			http.beginRequest();

			if (strcmp(metodo, "GET") == 0) {
				estadoHttp = http.get(resource);
			} else if (strcmp(metodo, "POST") == 0) {
				if (jsonData) {
					estadoHttp= http.post(resource/*, "application/json", jsonData*/);
					http.sendHeader("Content-Type", "application/json");
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
			http.print(jsonData);

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


			RegistroLogTarea reg;
			TickType_t espera = pdMS_TO_TICKS(10);
			respuestaHttp.toCharArray(reg.log, sizeof(reg.log));
			reg.tipoLog = 1; //http

			xQueueSend(colaRegistros, &reg, espera);


			cerrarConexionGPRS();

			//vTaskDelay(1000);
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

		if(respuesta.codigo == 200){
			guardarCadenaEE("SAAS_TOKEN", &respuesta.respuesta);
		}

		return respuesta.codigo;
	}



	RespuestaHttp postDatosSaas(String* modeloJson, SAAS_TIPO_HTTP_REQUEST tipoDatos){
		RespuestaHttp respuesta;
		const char* jsonData = modeloJson->c_str();

		if(tipoDatos == PAQUETE){
			 Serial.println("Envio de paquete");
			respuesta = realizarPeticionHttp("POST", postEventosJson, 1, jsonData);
		}else if(tipoDatos == NOTIFICACION){
			Serial.println("Envio de notificacion");
			respuesta = realizarPeticionHttp("POST", postNotificacionJson, 1, jsonData);
		}else {
			Serial.print("Error tipo de peticion http no reconocida");
		}

	    Serial.print("Codigo: ");
	    Serial.println(respuesta.codigo);
	    Serial.print("Contenido: ");
	    Serial.println(respuesta.respuesta);

		return respuesta;
	}


#endif /* SOURCE_ALARMAUTIL_HTTPREQUEST_H_ */
