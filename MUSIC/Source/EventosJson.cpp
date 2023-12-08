/*
 * EventosJson.cpp
 *
 *  Created on: 25 jun. 2023
 *      Author: Eduardo
 *
 *      Esta clase se encarga de registrar en el modelo JSON que se
 *      envia al SAAS, toda la informacion del sistema hasta la fecha
 *      del ultimo envio
 *
 *      Implementaciones:
 *
 *      Modelo accesible
 *      Mantenimiento en memoria NVS
 *      Generacion del modelo a enviar
 *      Purgado de los datos tras el envio
 *
 */

#include "EventosJson.h"

EventosJson::EventosJson() {
}


void EventosJson::iniciarModeloJSON() {

	//Restaurar modelo almacenado
	if (!cargarJsonNVS(JSON_DOC)) {
		Serial.println("MODELO JSON VACIO");
		componerJSON();
	}

}


JsonObject EventosJson::crearNuevaEntrada(char reg[]){


	// Acceder al array de entradas "Entry"
	JsonArray entry = JSON_DOC["Entry"];

	JsonObject entryObj = entry.createNestedObject();

	entryObj["isnew"] = true;
	entryObj["reg"] = reg;
	entryObj["date"] = fecha.imprimeFechaJSON();

	comprobarMemoriaDisponible();

	return entryObj;
}


void EventosJson::guardarEvento(char eventName[],char reg[]) {

	// Acceder al array de entradas "Entry"
	JsonArray entry = JSON_DOC["Entry"];

	if (entry.size() == 0) { //Si no hay entrada se crea a falso, es decir, ya existe en BBDD
		JsonObject entryObj = entry.createNestedObject();

		//Estos datos tienen que ser recuperados de anteriores ejecuciones
		char entradaInicial[10];

		snprintf(entradaInicial, sizeof(entradaInicial), "%d|%d|%d|%d",
						estadoAlarma,
						leerFlagEEInt("F_REACTIVACION"),
						leerFlagEEInt("F_RESTAURADO"),
						INTENTOS_REACTIVACION );

		entryObj["isnew"] = false;
		entryObj["reg"] = entradaInicial;
		entryObj["date"] = fecha.imprimeFechaJSON(); //Date es realmente necesario?

		//Estaba vacio por lo que creo una nueva entrada y de paso el sub array del Evento
		JsonArray event = entryObj.createNestedArray(eventName);

		//Ya podria insertar mi contenido
		JsonObject eventObj = event.createNestedObject();
		eventObj["reg"] = reg;
		eventObj["date"] = fecha.imprimeFechaJSON();

	} else {
		// Se encuentra el último elemento en el array "Entry"
		JsonObject lastEntry;
		for (JsonObject e : entry) {
			lastEntry = e;
		}

		//Compruebo que en la ultima entrada devuelta exista el sub array Detection
		JsonArray event;
		if (lastEntry.containsKey(eventName)) {
			event = lastEntry[eventName].as<JsonArray>();
		} else {
			event = lastEntry.createNestedArray(eventName);
		}

		//Ya podria insertar mi contenido
		JsonObject eventObj = event.createNestedObject();
		eventObj["reg"] = reg;
		eventObj["date"] = fecha.imprimeFechaJSON();

	}

	comprobarMemoriaDisponible();
}

 void EventosJson::componerJSON(){
	 Serial.println("Componiendo Modelo");
	 this->JSON_DOC = crearNuevoModeloJson();
	 guardarJsonNVS(JSON_DOC);
 }

 StaticJsonDocument<MAX_SIZE_JSON> EventosJson::crearNuevoModeloJson(){

	 JSON_DOC["version"] = version[0];
	 JSON_DOC["retry"] = "0";
	 JSON_DOC["id"] = "0";
	 JSON_DOC["date"] = fecha.imprimeFechaJSON(1);


	 JSON_DOC.createNestedArray("System");
	 JSON_DOC.createNestedArray("Entry");

	 // Acceder al último elemento del conjunto "System"
	 JsonArray ARRAY_SYSTEM = JSON_DOC["System"];

	 // Se crea un objeto "System" dentro del array "System"

	 //Trafico del modulo GSM
	 char numTrafic[20];
	 snprintf(numTrafic, sizeof(numTrafic), "%d|%d|%d|%d",
			 leerFlagEEInt("N_SMS_ENVIADOS"),
			 leerFlagEEInt("N_SYS_SEND"),
			 leerFlagEEInt("N_ALR_SEND"),
			 leerFlagEEInt("N_MOD_SEND"));

	 JsonObject E_ARR_SYSTEM = ARRAY_SYSTEM.createNestedObject();
	 E_ARR_SYSTEM["action"] = String(MODO_DEFAULT);
	 E_ARR_SYSTEM["msen"] = String(configSystem.MODO_SENSIBLE);
	 E_ARR_SYSTEM["alive"] = String(millis());
	 E_ARR_SYSTEM["traffic"] = numTrafic;
	 E_ARR_SYSTEM["gsm"] = String(getCalidadRed()) + "|" + String(getVoltajeBateria());
	 E_ARR_SYSTEM["modules"] = String(configSystem.MODULO_SD)+
			 "|" + String(configSystem.MODULO_RTC)+
			 "|0";
	 E_ARR_SYSTEM["sensors"] = 	"102;"+String(configSystem.SENSORES_HABLITADOS[0])+
			 "|103;"+String(configSystem.SENSORES_HABLITADOS[1])+
			 "|104;"+String(configSystem.SENSORES_HABLITADOS[2])+
			 "|105;"+String(configSystem.SENSORES_HABLITADOS[3]);
	 E_ARR_SYSTEM["reset"] = fecha.imprimeFechaJSON(0, fecha.getFechaReset());

	 return JSON_DOC;
 }


void EventosJson::purgarModeloJSON(){

	Serial.print("Purgando Modelo: ");
	JSON_DOC.clear();
	SALIDA_JSON = "";
	Serial.println(JSON_DOC.memoryUsage());

}

void EventosJson::mostrarModeloJSON(){

	// Se serializa el objeto JSON a una cadena de caracteres
	//serializeJson(JSON_DOC, SALIDA_JSON);
	SALIDA_JSON.clear();
	serializeJsonPretty(JSON_DOC, SALIDA_JSON);

	//Se imprime el resultado en el monitor serie
	Serial.println(SALIDA_JSON);
	SALIDA_JSON.clear();
    Serial.println(JSON_DOC.memoryUsage());


}

void EventosJson::guardarEntrada(){

	char reg[10];

	snprintf(reg, sizeof(reg), "%d|%d|%d|%d",
					estadoAlarma,
					leerFlagEEInt("F_REACTIVACION"),
					leerFlagEEInt("F_RESTAURADO"),
					INTENTOS_REACTIVACION );
	crearNuevaEntrada(reg);

}

void EventosJson::guardarDeteccion(byte strikes, byte umbral, byte modo, byte idTerminal, byte id, byte estado,  byte valor){

/*
	'intrusismo' => Si la deteccion es la que provoca intrusismo
	'umbral' => Rango maximo
	'restaurado' => Si se ha restaurado la detecccion
	'modo_deteccion' =>  "normal" : "phantom",
	id_terminal idTerminal
	'tipo' => ID sensor,
	'estado' => "ONLINE" : "OFFLINE",
	'valor_sensor' => VALOR DIGITAL O ANALOGICO,
  */

	char reg[20];
	byte sensor_id[] = {103, 104, 105,102};

	snprintf(reg, sizeof(reg), "%d|%d|%d|%d|%d|%d|%d|%d",
			(strikes==umbral),
			umbral,
			0,
			modo,
			(idTerminal == 0)? 0 : idTerminal,
			(idTerminal == 0)? sensor_id[id] : id,
			estado,
			valor);

	guardarEvento("Detection",reg);

}

void EventosJson::guardarNotificacion(byte tipo, byte asunto, char cuerpo[],byte tlf){

	char reg[1024]; //Para que entre el log

	// Limitar el campo cuerpo a 800 caracteres
	char cuerpoLimitado[801];
	strncpy(cuerpoLimitado, cuerpo, 800);
	cuerpoLimitado[800] = '\0';

	snprintf(reg, sizeof(reg), "%d|%d|%s|%d",
			tipo,
			asunto, //0 = Llamadas
			cuerpoLimitado, //Cadena vacia = Llamadas
			tlf);

	guardarEvento("Notice",reg);

}

void EventosJson::guardarLog(byte id){

	char reg[5];
	snprintf(reg, sizeof(reg), "%d",id);

	guardarEvento("Log",reg);
}

void EventosJson::comprobarMemoriaDisponible(){

	if(JSON_DOC.memoryUsage() >= (MAX_SIZE_JSON-400)){
		Serial.println("JSON Overload exportando a fichero");
		this->exportarFichero();
	}else {
		//Si la memoria no ha sido superada actualizo en NVS
		guardarJsonNVS(JSON_DOC);
	}

}

void EventosJson::exportarFichero(){

	actualizarCabecera();
	//Añadir ID a cadena para tratar
	if(!registro.exportarEventosJson(&JSON_DOC)){
		Serial.println(F("Sin acceso a SD el modelo se purga"));
	}

	//Vaciar memoria JSON
	purgarModeloJSON();
	//Preparamos el modelo vacio
	componerJSON();

}

void EventosJson::guardarJsonNVS(){
	guardarJsonNVS(JSON_DOC);
}

void EventosJson::cargarJsonNVS(){
	cargarJsonNVS(JSON_DOC);
}



void EventosJson::guardarJsonNVS(StaticJsonDocument<MAX_SIZE_JSON>& jsonDoc) {
	// Serializar el documento JSON en un String
	String jsonString;
	serializeJson(jsonDoc, jsonString);

	//Serial.print("Guardando Modelo: ");
	//Serial.println(jsonString);

	NVSMemory.begin("SAA_DATA", false);
	NVSMemory.putString("MODELO_JSON", jsonString);
	NVSMemory.end();

}

byte EventosJson::cargarJsonNVS(StaticJsonDocument<MAX_SIZE_JSON>& jsonDoc) {

	NVSMemory.begin("SAA_DATA", false);
	String jsonString = NVSMemory.getString("MODELO_JSON");
	NVSMemory.end();

	//Serial.print("Cargando Modelo: ");
	//Serial.println(jsonString);

	if(jsonString.isEmpty() || jsonString.c_str() == nullptr){
		return 0;
	}

	// Deserializar el String en el documento JSON
	DeserializationError error = deserializeJson(jsonDoc, jsonString);

	// Test if parsing succeeds.
	if (error) {
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.f_str());
		return 0;
	}

	if (jsonDoc.size() == 0) {
		Serial.println("Modelo JSON vacio Error desde memoria");
		return 0;
	} else {
		Serial.println("Modelo JSON cargado desde memoria OK");
	}

	return 1;
}


void EventosJson::actualizarCabecera(){

	JSON_DOC["date"] = fecha.imprimeFechaJSON(1);

	//Recuperamos los datos del sistema
	JsonArray system = JSON_DOC["System"];
	JsonObject lastSystem;

	for (JsonObject e : system) {
		lastSystem = e;
	}

	//Trafico del modulo GSM
	char numTrafic[20];
	snprintf(numTrafic, sizeof(numTrafic), "%d|%d|%d|%d",
			leerFlagEEInt("N_SMS_ENVIADOS"),
			leerFlagEEInt("N_SYS_SEND"),
			leerFlagEEInt("N_ALR_SEND"),
			leerFlagEEInt("N_MOD_SEND"));

	lastSystem["action"] = String(MODO_DEFAULT);
	lastSystem["msen"] = String(configSystem.MODO_SENSIBLE);
	lastSystem["alive"] = String(millis());
	lastSystem["traffic"] = numTrafic;
	lastSystem["gsm"] = String(getCalidadRed()) + "|" + String(getVoltajeBateria());
	lastSystem["modules"] = String(configSystem.MODULO_SD)+
			"|" + String(configSystem.MODULO_RTC)+
			"|0";
	lastSystem["sensors"] = 	"102;"+String(configSystem.SENSORES_HABLITADOS[0])+
			"|103;"+String(configSystem.SENSORES_HABLITADOS[1])+
			"|104;"+String(configSystem.SENSORES_HABLITADOS[2])+
			"|105;"+String(configSystem.SENSORES_HABLITADOS[3]);
	lastSystem["reset"] = fecha.imprimeFechaJSON(0,fecha.getFechaReset());

}

String EventosJson::asignarIdPaquete(String* modelo){

	int ultimoIdInstalado = leerFlagEEInt("PACKAGE_ID");
	ultimoIdInstalado++;
	Serial.print(F("Vinculando modelo a Id:"));
	Serial.println(ultimoIdInstalado);
	return registro.actualizarIdModelo(modelo, ultimoIdInstalado);
}


void EventosJson::confirmarIdPaquete(){
	/*Envio OK avanzamos ID*/
	int ultimoIdInstalado = leerFlagEEInt("PACKAGE_ID");
	ultimoIdInstalado++;
	Serial.print(F("Id instalado en servidor, ultimo Id:"));
	Serial.println(ultimoIdInstalado);
	guardarFlagEE("PACKAGE_ID", ultimoIdInstalado);
}

SAAS_GESTION_ENVIO_R EventosJson::gestionarEnvioModeloJson(String* modeloJson, SAAS_TIPO_HTTP_REQUEST tipoDatos){

	SAAS_GESTION_ENVIO_R resultado = ERROR_ENVIO;
	byte flagSalida = 0;
	byte reintentosPost = 0;

	String modelo = *modeloJson;
	RespuestaHttp respuesta;

	gestionPaquete = ENVIAR_POR_POST;

	while(flagSalida == 0){
		switch (gestionPaquete) {
		case ENVIAR_POR_POST:

			respuesta = postDatosSaas(&modelo, tipoDatos);
			switch (respuesta.codigo) {
			case 200:
				//Salir del bucle si ok
				Serial.println(F("200 Paquete enviado OK"));
				resultado = ENVIO_OK;
				flagSalida++;
				break;
			case 400:
				Serial.println(F("Error 400"));
				gestionPaquete = PROCESAR_RESPUESTA_ERROR;
				break;
			case 401:
				Serial.println(F("Error 401"));
				gestionPaquete = ACTUALIZAR_TOKEN;
				break;
			default:
				Serial.println(F("Err 500 aborto"));
				gestionPaquete = ABORTAR_ENVIO;
				break;
			}
			break;

			case PROCESAR_RESPUESTA_ERROR:
				if (respuesta.respuesta.equals("Id de paquete duplicado")) {
					//Si el error es por el id volvemos a consultarlo
					if (getIdPaqueteSaas() != 200) {
						gestionPaquete = ABORTAR_ENVIO;
						break;
					}
					resultado = ERROR_ID;
					flagSalida++;
					break;

				} else {
					gestionPaquete = ABORTAR_ENVIO;
				}

				break;

			case ACTUALIZAR_TOKEN:

				if (generarTokenSaas() == 200) {
					reintentosPost++;
					if(reintentosPost > 1){
						gestionPaquete = ABORTAR_ENVIO;
						break;
					}
					Serial.println(F("Token generado ok"));

					gestionPaquete = ENVIAR_POR_POST;
				} else {
					gestionPaquete = ABORTAR_ENVIO;
				}

				break;

			case ABORTAR_ENVIO:
				Serial.println("Abortando el envio...");
				//Salir
				flagSalida++;
				break;
		}

	}

	return resultado;
}

byte EventosJson::enviarInformeSaas(){

	Serial.println(F("Enviando informe a SAAS"));
	SAAS_GESTION_ENVIO_R resultado;
	byte estadoEnvio  = 0;


	RegistroLogTarea reg;
	reg.tipoLog = 0; //systema


	//Comprobar si existen paquetes exportados
	if(configSystem.MODULO_SD || SD_STATUS == 1){

		//Se procesa el modelo almacenado
		Serial.println(F("Comprobamos si existen modelos pendientes"));
		String modelo;
		//Comprobar si existen paquetes exportados
		while(true){
			modelo = registro.leerPrimerElemento();

			if(modelo.isEmpty() || modelo.c_str() == nullptr){
				Serial.println(F("No quedan paquetes pendientes"));
				break;
			}

			Serial.println(F("Procesando modelo pendiente"));

			//Comprobamos si el modelo tiene demasiados reintentos en cuyo caso se eliminara
			if(registro.leerReintentosModelo(&modelo) == MAX_REINTENTOS_ENVIO_MODELO){
				Serial.println(F("Modelo eliminado por exceso de reintentos"));
				snprintf(reg.log, sizeof(reg.log), "Modelo eliminado por exceso de reintentos");
				reg.saasLogid = MODELO_ELIMINADO_EXCESO_REINTENTOS_LOG;

				xQueueSend(colaRegistros, &reg, 0);

				registro.extraerPrimerElemento(); //Descartando modelo
			}

			modelo = asignarIdPaquete(&modelo);

			Serial.println(modelo);

			resultado = gestionarEnvioModeloJson(&modelo, PAQUETE);

			if(resultado == ENVIO_OK){
				Serial.println(F("Modelo sd enviado"));
				confirmarIdPaquete();
				registro.extraerPrimerElemento(); //Saco el registro

				 snprintf(reg.log, sizeof(reg.log), "Modelo enviado desde SD");
				 reg.saasLogid = MODELO_ENVIADO_SD_LOG;
				 xQueueSend(colaRegistros, &reg, 0);

				estadoEnvio = 1;
			}else if(resultado == ERROR_ID){

				 snprintf(reg.log, sizeof(reg.log), "Error en el id del modelo");
				 reg.saasLogid = MODELO_ERROR_EN_ID_LOG;
				 xQueueSend(colaRegistros, &reg, 0);


				estadoEnvio = 0;
				return estadoEnvio; //Salgo
			}
			else {
				//Error abortar informe
				registro.actualizarUltimoElemento("retry");

				 snprintf(reg.log, sizeof(reg.log), "Error enviando modelo");
				 reg.saasLogid = MODELO_ERROR_ENVIO_LOG;
				 xQueueSend(colaRegistros, &reg, 0);

				estadoEnvio = 0;
				return estadoEnvio; //Salgo
			}

		}
	}

	//Se procesa el modelo en memoria
	Serial.println(F("Procesando modelo en memoria"));

	actualizarCabecera();
	serializeJson(JSON_DOC, SALIDA_JSON);
	Serial.print("MODELO ACTUAL -> ");

	SALIDA_JSON = asignarIdPaquete(&SALIDA_JSON);
	Serial.println(SALIDA_JSON);

	resultado = gestionarEnvioModeloJson(&SALIDA_JSON, PAQUETE);

	if(resultado != ENVIO_OK){
		estadoEnvio = 0;
		//Si tengo acceso a SD guardo la informacion
		if(configSystem.MODULO_SD || SD_STATUS == 1){
			JSON_DOC["retry"] = "1";

			if (resultado == ERROR_ID) {

				snprintf(reg.log, sizeof(reg.log), "Error en el id del modelo");
				reg.saasLogid = MODELO_ERROR_EN_ID_LOG;
				xQueueSend(colaRegistros, &reg, 0);

			}

			if (resultado == ERROR_ENVIO) {
				//Error el modelo actual a fallado por lo que es enviado a fichero para su posterior reenvio

				snprintf(reg.log, sizeof(reg.log), "Error enviando modelo");
				reg.saasLogid = MODELO_ERROR_ENVIO_LOG;
				xQueueSend(colaRegistros, &reg, 0);

			}

			registro.exportarEventosJson(&JSON_DOC);
		}

	}else {
		Serial.println(F("Modelo memoria enviado"));
		confirmarIdPaquete();

		snprintf(reg.log, sizeof(reg.log), "Modelo enviado");
		//reg.saasLogid = MODELO_ENVIADO_LOG; //No enviar copia al siguente modelo redundante
		xQueueSend(colaRegistros, &reg, 0);

		estadoEnvio = 1;
	}


	SALIDA_JSON.clear();
	//Vaciar memoria JSON
	purgarModeloJSON();
	//Preparamos el nuevo modelo vacio
	componerJSON();

	return estadoEnvio;

}


byte EventosJson::enviarNotificacionSaas(byte tipo, const char* contenido){
	//Se prepara una notificacion para enviar al servidor

	SAAS_GESTION_ENVIO_R resultado;

	StaticJsonDocument<300> notificacion;

	notificacion["type"] = tipo;
	notificacion["reg"] = contenido;
	notificacion["date"] = fecha.imprimeFechaJSON(1);

	String notificacionModelo;
	serializeJson(notificacion, notificacionModelo);

	Serial.println(notificacionModelo);

	resultado = gestionarEnvioModeloJson(&notificacionModelo, NOTIFICACION);


	RegistroLogTarea reg;

	snprintf(reg.log, sizeof(reg.log), "[%s] - Notificacion %s contenido: %s",
			((resultado == ENVIO_OK) ? "Notificacion enviada con exito" : "Error notificacion no enviada"),
			(tipo ? "del sistema" : "de alarma"),
			//contenido->c_str()
			contenido
			);
	reg.tipoLog = 0; //systema

	xQueueSend(colaRegistros, &reg, 0);


	return (resultado == ENVIO_OK);

}
