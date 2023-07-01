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
	//TODO comprobar previamiente si hay algo
	JSON_DOC = NVS_RestoreDataJSON<StaticJsonDocument<MAX_SIZE_JSON>>("MODELO_JSON");

	// Comprueba si el documento está vacío
	if (JSON_DOC.isNull()) {
	  Serial.println("MODELO JSON VACIO");
	  crearNuevoModeloJson();
	  NVS_SaveDataJSON<StaticJsonDocument<MAX_SIZE_JSON>>("MODELO_JSON", JSON_DOC);
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
	 //Actualizar el ID
	 guardarFlagEE("JSON_SEQ", (leerFlagEEInt("JSON_SEQ")+1));
	 crearNuevoModeloJson();

	 //TODO actualizar los datos de la cabecera
 }

 StaticJsonDocument<MAX_SIZE_JSON> EventosJson::crearNuevoModeloJson(){

	 JSON_DOC["version"] = "VE21R0";
	 JSON_DOC["retry"] = String(leerFlagEEInt("JSON_RETRY"));
	 JSON_DOC["id"] = String(leerFlagEEInt("JSON_SEQ"));
	 JSON_DOC["date"] = fecha.imprimeFechaJSON();

	 JSON_DOC.createNestedArray("System");
	 JSON_DOC.createNestedArray("Entry");

	 // Acceder al último elemento del conjunto "System"
	 JsonArray ARRAY_SYSTEM = JSON_DOC["System"];

	 // Se crea un objeto "System" dentro del array "System"
	 JsonObject E_ARR_SYSTEM = ARRAY_SYSTEM.createNestedObject();
	 E_ARR_SYSTEM["action"] = String(MODO_DEFAULT);
	 E_ARR_SYSTEM["msen"] = String(configSystem.MODO_SENSIBLE);
	 E_ARR_SYSTEM["alive"] = String(millis());
	 E_ARR_SYSTEM["numsms"] = String(leerFlagEEInt("N_SMS_ENVIADOS"));
	 E_ARR_SYSTEM["modules"] = String(configSystem.MODULO_SD)+
			 "|" + String(configSystem.MODULO_RTC)+
			 "|0";
	 E_ARR_SYSTEM["sensors"] = 	"102;"+String(configSystem.SENSORES_HABLITADOS[0])+
			 "|103;"+String(configSystem.SENSORES_HABLITADOS[1])+
			 "|104;"+String(configSystem.SENSORES_HABLITADOS[2])+
			 "|105;"+String(configSystem.SENSORES_HABLITADOS[3]);
	 E_ARR_SYSTEM["reset"] = fecha.imprimeFechaJSON(fecha.getFechaReset());

	 return JSON_DOC;
 }


void EventosJson::purgarModeloJSON(){

	JSON_DOC.clear();
	SALIDA_JSON = "";
	Serial.println(JSON_DOC.memoryUsage());

}

void EventosJson::mostrarModeloJSON(){

	// Se serializa el objeto JSON a una cadena de caracteres
	//serializeJson(JSON_DOC, SALIDA_JSON);
	SALIDA_JSON = "";
	serializeJsonPretty(JSON_DOC, SALIDA_JSON);

	//Se imprime el resultado en el monitor serie
	Serial.println(SALIDA_JSON);

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

void EventosJson::guardarDeteccion(byte strikes, byte umbral, byte modo, byte id, byte estado,  byte valor){

/*
	'intrusismo' => Si la deteccion es la que provoca intrusismo
	'umbral' => Rango maximo
	'restaurado' => Si se ha restaurado la detecccion
	'modo_deteccion' =>  "normal" : "phantom",
	'tipo' => ID sensor,
	'estado' => "ONLINE" : "OFFLINE",
	'valor_sensor' => VALOR DIGITAL O ANALOGICO,
  */

	char reg[20];
	byte sensor_id[] = {102, 103, 104, 105};

	snprintf(reg, sizeof(reg), "%d|%d|%d|%d|%d|%d|%d",
			(strikes==umbral),
			umbral,
			0, //Se restauran detecciones?
			modo,
			sensor_id[id],
			estado,
			valor);

	guardarEvento("Detection",reg);

}

void EventosJson::guardarNotificacion(byte tipo, byte asunto, char cuerpo[],byte tlf){
	/*
	        'tipo' => ($noticeField[0]) ? "sms" : "llamada",
                                'asunto' => ($noticeField[1]),
                                'cuerpo' => ($noticeField[2]),
                                'telefono' => ($noticeField[3]),
	 	guardarEvento("Notice","1|116|Pendiente|2");
	 */
	char reg[200];

	snprintf(reg, sizeof(reg), "%d|%d|%s|%d",
			tipo,
			asunto, //0 = Llamadas
			cuerpo, //Cadena vacia = Llamadas
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
	}

}

void EventosJson::exportarFichero(){

	//Añadir ID a cadena para tratar
	if(!registro.exportarEventosJson(&JSON_DOC)){
		//TODO Si no se puede guardar se envia directamente
	}

	//Vaciar memoria JSON
	purgarModeloJSON();
	//Preparamos el modelo vacio
	componerJSON();

}

void EventosJson::guardarJsonNVS(){

	NVS_SaveDataJSON<StaticJsonDocument<MAX_SIZE_JSON>>("MODELO_JSON", JSON_DOC);
}
