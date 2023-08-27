/*
 * Terminal.cpp
 *
 *  Created on: 8 jul. 2023
 *      Author: isrev
 *
 *
 *  Esta clase se encarga de almacenar los datos
 *  relativos a los terminales conectados a la linea
 *
 */

#include "Terminal.h"

Terminal::Terminal(byte id, char* nombreTerminal, byte numFotoSensor, byte numLineasCtl, byte numSensores, int* dimensionSamplesMapeo) { // @suppress("Class members should be properly initialized")

	this->TERMINAL_ID = id;
	strncpy(TERMINAL_NAME, nombreTerminal, sizeof(TERMINAL_NAME) - 1);
	TERMINAL_NAME[sizeof(TERMINAL_NAME) - 1] = '\0';  // Asegurarse de que el arreglo tenga un carácter nulo al final

	this->NUM_FOTO_SENSOR = numFotoSensor;
	this->NUM_LINEAS_CTL = numLineasCtl;
	this->NUM_SENSORES = numSensores;

	this->DATOS_FOTOSENSOR = 0;
	this->BAD_REPLY_STRIKE = 0;
	this->NO_REPLY_STRIKE = 0;
	this->BAD_COMM_STRIKE = 0;

	this->listaTerminal.cabeza = NULL;
	this->listaTerminal.longitud = 0;

	//Se definen las dimensiones por sensor
	if (dimensionSamplesMapeo != nullptr) {
		for (int i = 0; i < numSensores; i++) {
			this->dimensionSamplesMapeo[i] = dimensionSamplesMapeo[i];
		}
	}else {
		for (int i = 0; i < MAX_DATOS_SUB_TRAMA; i++) {
			this->dimensionSamplesMapeo[i] = FRACCION_SALTO;
		}
	}
}


byte Terminal::getTerminalId() const {
	return TERMINAL_ID;
}

int Terminal::getDatosFotosensor() const {
	return DATOS_FOTOSENSOR;
}


byte Terminal::getNumFotoSensor() const {
	return NUM_FOTO_SENSOR;
}


byte Terminal::getNumLineasCtl() const {
	return NUM_LINEAS_CTL;
}


byte Terminal::getNumSensores() const {
	return NUM_SENSORES;
}


const char* Terminal::getTerminalName() const {
	return TERMINAL_NAME;
}

void Terminal::setDatosFotosensor(int datosFotosensor) {
	DATOS_FOTOSENSOR = datosFotosensor;
}

unsigned int Terminal::getBadReplyStrike() const {
	return BAD_REPLY_STRIKE;
}


unsigned int Terminal::getNoReplyStrike() const {
	return NO_REPLY_STRIKE;
}

void Terminal::addBadReplyStrike() {
	BAD_REPLY_STRIKE = BAD_REPLY_STRIKE++;
}

void Terminal::addNoReplyStrike() {
	NO_REPLY_STRIKE = NO_REPLY_STRIKE++;
}


unsigned int Terminal::getBadCommStrike() const {
	return BAD_COMM_STRIKE;
}

void Terminal::addBadCommStrike() {
	BAD_COMM_STRIKE = BAD_COMM_STRIKE++;
}

void Terminal::limpiarStrikes(){
	BAD_REPLY_STRIKE = 0;
	NO_REPLY_STRIKE = 0;
	BAD_COMM_STRIKE = 0;
}



void Terminal::guardarDatosTerminal(byte* arrSensorSamples, byte* arrControlLineas){
	Serial.println("Guardando datos");

	Data datosNodo;

	//Determino si el array contiene algo sino se descarta el nodo
	if(arrSum<byte>(arrSensorSamples, MAX_DATOS_SUB_TRAMA)){

		//Gestion nodos
		controlNodosEnMemoria();
		//Se eliminan los nodos mayores
		purgarNodosViejos(&this->listaTerminal);

		for (int i = 0; i < MAX_DATOS_SUB_TRAMA; i++) {
			datosNodo.sampleSensores[i] = arrSensorSamples[i];
		}
		datosNodo.marcaTiempo = millis();
		this->InsertarFinal(&this->listaTerminal, datosNodo);
	}

	//Guardo la informacion sobre el estado del servicio
	for (int i = 0; i < MAX_DATOS_CTL_LINEA; i++) {
		datosControlLineas[i] = arrControlLineas[i];
	}

}

void Terminal::recorrerDatosTerminal(){
	this->RecorrerLista(&this->listaTerminal);
}

void Terminal::borrarPrimerElemento(){
	this->EliminarPrincipio(&this->listaTerminal);
}

void Terminal::borrarUltimoElemento(){
	this->EliminarUltimo(&this->listaTerminal);
}

void Terminal::controlNodosEnMemoria(){
	//Se controla que no pueda haber un desbordamiento
	if (listaLongitud(&this->listaTerminal) > MAX_NODOS_EN_EJECUCION) {
		EliminarPrincipio(&this->listaTerminal);
	}

}

RespuestaTerminal Terminal::evaluarDatosTerminal(){

	byte flagEstadoServicio = 0;
	byte flagFotoSensor = 0;
	byte flagDeteccion = 0;

    char registroConjunto[50];

	respuesta.idTerminal = getTerminalId(); //identificamos la respuesta

	//Evalua el estado del servicio si existen i o superan los umbrales de strike
	if(getNoReplyStrike() >= UMBRAL_NO_REPLY_STRIKE){
		respuesta.interpretacion = NO_REPLY;

		Serial.println("INTER: NO REPLY");
		return respuesta;
	}
	if(getBadCommStrike() >= UMBRAL_BAD_COMM_STRIKE){
		respuesta.interpretacion = BAD_COMM;
		Serial.println("INTER: BAD COMM");
		return respuesta;
	}

	if(getBadReplyStrike() >= UMBRAL_BAD_REPLY_STRIKE){
		respuesta.interpretacion = BAD_REPLY;
		Serial.println("INTER: BAD REPLY");
		return respuesta;
	}


	//Evalua los datos estaticos
		//Comprobamos los controles de linea
	for (int i = 0; i < getNumLineasCtl(); i++) {
		//hay que determinar si existen porcentajes altos en sesores para deteminar averia o sabotaje
		if(datosControlLineas[i]){
			respuesta.resumen = "";
			respuesta.resumen += "LINEA " + String(i);
			flagEstadoServicio = 1;
		}
	}

	//Comprobamos los valores del sensor fotosensible
	if(getNumFotoSensor() && getDatosFotosensor() > UMBRAL_FOTORESISTENCIA){
		persistenciaFotoresistencia++;

		if(persistenciaFotoresistencia > UMBRAL_PERSISTENCIA_FOTORESISTENCIA)
		flagFotoSensor =1;
	}

	//Evalua los datos en la lista
	if(!listaVacia(&this->listaTerminal)){
		//Se eliminan los nodos mayores
		purgarNodosViejos(&this->listaTerminal);
		recalcularNumDeteciones(&this->listaTerminal);

		//Revisamos si hay nuevos nodos
		if(nodosRevisados < listaLongitud(&this->listaTerminal)){
			//Se itera por cada sensor el contenido de la lista
			 porcentajeDeteccion = 0.0;

			for (int i = 0; i < getNumSensores(); i++) {
				double sensorPorcentaje = EvaluarSensor(&listaTerminal, i);

				if(sensorPorcentaje>0){

					Serial.print("SENSOR: ");
					Serial.print(i+1);
					Serial.print(" INTER: %: ");
					Serial.println(sensorPorcentaje);

					snprintf(registroConjunto, sizeof(registroConjunto), "%s%d%s", "MOVIMIENTO EN TERMINAL X:",i," ONLINE");
					registro.registrarLogSistema(registroConjunto);
					eventosJson.guardarDeteccion((sensorPorcentaje >= UMBRAL_SENSOR_INDIVIDUAL),
													FRACCION_SALTO,
													P_MODO_NORMAL,
													getTerminalId(),
													i,
													P_ESTADO_ONLINE); //Regularizar el id del terminal
				}

				if(sensorPorcentaje >= UMBRAL_SENSOR_INDIVIDUAL){
					flagDeteccion =1;
					Serial.println("Umbral de salto individual");
					respuesta.resumen = "";
					respuesta.resumen += "S" + String(i+1) + ":" + String(sensorPorcentaje) + "%";

					respuesta.idSensorDetonante = i;
				}
				porcentajeDeteccion += sensorPorcentaje;
			}

			if(porcentajeDeteccion >= UMBRAL_SENSOR_TOTAL){
				flagDeteccion =1;
				Serial.println("Umbral de salto total del terminal");
				respuesta.resumen = "";
				respuesta.resumen += "T" + String(porcentajeDeteccion) + "%";
			}

			nodosRevisados = listaLongitud(&this->listaTerminal);
		}

	}

	if(flagEstadoServicio && porcentajeDeteccion > UMBRAL_SABOTAJE){
		respuesta.interpretacion = SABOTAJE;
		Serial.println("INTER: SABOTAJE");
	}else if(flagEstadoServicio){
		respuesta.interpretacion = AVERIA;
		Serial.println("INTER: AVERIA");
	}else if(flagDeteccion){
		respuesta.interpretacion = DETECCION;
		Serial.println("INTER: DETECCION");
	}else if(flagFotoSensor){
		Serial.println("INTER: FOTO");
		respuesta.interpretacion = DETECCION_FOTOSENIBLE;
	}
	else {
		respuesta.interpretacion = TERMINAL_OK;
	}

	if(respuesta.interpretacion != TERMINAL_OK){
		//La respuesta va a detonar un accion, reseteamos las variables a su estado original
		nodosRevisados = 0;
		porcentajeDeteccion = 0.0;
	}

	return respuesta;
}

void Terminal::evaluarPhantomTerminal(){
	EvaluarSensorPhantom(&listaTerminal);
}


void Terminal::limpiarDatosTerminal(){
	//Eliminamos los nodos del terminal
	purgarLista();
	//Reinicializamos el estado del servicio
	for (int i = 0; i < MAX_DATOS_CTL_LINEA; i++) {
		datosControlLineas[i] = 0;
	}
	//Eliminamos los datos de los contadores
	recalcularNumDeteciones(&this->listaTerminal);

	setDatosFotosensor(0);
	limpiarResultadoPhantom();
	limpiarStrikes();

	//Reset de variables
	nodosRevisados = 0;
    porcentajeDeteccion = 0.0;

}

String Terminal::generarInformeDatos(){

	String salida;
	Nodo* puntero = this->listaTerminal.cabeza;
	unsigned long marcaTiempoNodoAnterior = 0;
	double porcentajeTotal = 0.0;

	//Evalua los datos en la lista
	if(!listaVacia(&this->listaTerminal)){
		String t = std::string(((getNumSensores()*8)-2), '-').c_str(); //62
		salida += "\n-Terminal: ";
		salida += getTerminalName();
		salida += "\n";

		//pintar cabecera
		salida += generarCabecera(getNumSensores());
		salida += t;
		salida += "\n";

		//Pintar nodos
		while(puntero){
			for (int i = 0; i < getNumSensores(); i++) {
				salida += puntero->data.sampleSensores[i];
			    salida += "\t\t";
			}

			if(marcaTiempoNodoAnterior == 0){
				 salida += "\tseg: 0\n";
			}else {
				salida += "\tseg: " + String((puntero->data.marcaTiempo-marcaTiempoNodoAnterior)/1000) + "\n";
			}
			marcaTiempoNodoAnterior = puntero->data.marcaTiempo;

			puntero = puntero->siguente; //Muevo el iterador una posicion
		}
		salida += t;
		salida += "\n";

		//Pintar resultado analisis
		for (int i = 0; i < getNumSensores(); i++) {
			double d = EvaluarSensor(&listaTerminal, i);
			porcentajeTotal += d;
			salida += String(d);
			salida += "%\t";
		}

		salida += "\n";
		salida += String(t);
		salida += "\n";
		salida += "Total: ";
		salida += String(porcentajeTotal);
		salida += "% Max:";
		salida += UMBRAL_SENSOR_TOTAL;
		salida += "%\n";

		//Pintar datos fotoresistencias
		if(getNumFotoSensor() &&  (persistenciaFotoresistencia > UMBRAL_PERSISTENCIA_FOTORESISTENCIA)){
			salida += "Lectura luminica: ";
            salida += String(getDatosFotosensor());
		}
		//Pintar detalles del servicio
		for (int i = 0; i < getNumLineasCtl(); i++) {
			if(datosControlLineas[i]){
				salida += "Averia en la linea de datos: ";
				salida += String(i+1);
				salida += "\n";
			}
		}

		salida += "\n";
	}else {
		salida += "No detectada presencia en el terminal ";
		salida += getTerminalName();
		salida += "\n";
	}
	return salida;
}

void Terminal::limpiarResultadoPhantom(){
	for (int i = 0; i < getNumSensores(); i++) {
		sampleSensoresPhantom[i] = 0;
	}
	maxEjecucion = 0;
}

String Terminal::generarCabecera(int numElementos) {
    String salida;

    salida += '(';

    for (int i = 1; i <= numElementos; i++) {
    	salida += String(literalesZonas[getTerminalId()][i-1]);

    	if(i+1 <= numElementos)
    	salida += ", ";
    }

	salida += ")\n";

    for (int i = 1; i <= numElementos; i++) {
    	salida += "S0" + String(i) +"\t";
        salida += '\t';
    }
    salida += '\n';
    return salida;
}

 int Terminal::getSampleSensores(int numSensor) {
	return sampleSensoresCont[numSensor];
}

void Terminal::purgarLista(){
	int t = listaLongitud(&this->listaTerminal);
	for (int i = 0; i < t; i++){
		EliminarPrincipio(&this->listaTerminal);
	}
}

double Terminal::calcularPorcentaje(int nSaltos, int nMatch, int maxMatchConsecutivo) {
    return ((nSaltos * PORCENTAJE_SALTO_UNITARIO) + ((nMatch * 100.0) / static_cast<double>(maxMatchConsecutivo)));
}

double Terminal::EvaluarSensor(Lista* lista, int numSensor) {

	if(lista->cabeza == NULL /*|| lista->cabeza->siguente == NULL*/){
		return NULL;
	}

    Nodo* puntero = lista->cabeza;
    int contadorMatch = 0;
    int contadorSalto = 0;
    unsigned long diferenciaTiempo = 0;
    unsigned long diferenciaTiempoAux = 0;


    while (puntero) {
        if (puntero->data.sampleSensores[numSensor]) {
        	contadorSalto++;
            if (diferenciaTiempoAux == 0) {
                diferenciaTiempoAux = puntero->data.marcaTiempo;
                contadorMatch++; //Un primer combo para aumentar el porcentaje al principio

            } else {
                diferenciaTiempo = puntero->data.marcaTiempo - diferenciaTiempoAux;
                diferenciaTiempoAux = puntero->data.marcaTiempo;

                if (diferenciaTiempo <= TIEMPO_COMBO) {
                    contadorMatch++;
                } else {
                    contadorMatch = 0;
                }
            }
        }else {
        	//COMO GESTIONAMOS ESTO?
          if((puntero->data.marcaTiempo - diferenciaTiempo) > TIEMPO_COMBO)
           contadorMatch = 0;
        }
        puntero = puntero->siguente;
    }

    sampleSensoresCont[numSensor] = contadorSalto; //Controlamos el numero por salto

    return  calcularPorcentaje(contadorSalto, contadorMatch, dimensionSamplesMapeo[numSensor]);
}


void Terminal::EvaluarSensorPhantom(Lista* lista){

    if (lista->cabeza == NULL) {
        return;
    }

    Nodo* puntero = lista->cabeza;
    char registroConjunto[50];

    while (puntero) {
        if(puntero->data.marcaTiempo > maxEjecucion){
            for (int i = 0; i < getNumSensores(); i++) {
                 if(puntero->data.sampleSensores[i]){

                	 snprintf(registroConjunto, sizeof(registroConjunto), "%s%d%s", "MOVIMIENTO PHANTOM EN TERMINAL X:",i," ONLINE");
                	 registro.registrarLogSistema(registroConjunto);
                	 eventosJson.guardarDeteccion(
                			 1,
							 FRACCION_SALTO,
							 P_MODO_PHANTOM,
							 getTerminalId(),
							 i,
							 P_ESTADO_ONLINE); //Regularizar el id del terminal
                     sampleSensoresPhantom[i]++;
                }
            }
           maxEjecucion = puntero->data.marcaTiempo;
        }
        puntero = puntero->siguente;
    }

}


void Terminal::recalcularNumDeteciones(Lista* lista){
	/*Se recaulculan los saltos por sensor cuando un nodo se elimina de la lista*/
	Nodo* puntero = lista->cabeza;
	static int numNodos;

	if(numNodos != lista->longitud){
		numNodos = lista->longitud;

		//Refresco contador
		for (int i = 0; i < getNumSensores(); i++) {
			sampleSensoresCont[i] = 0;
		}

		while (puntero) {
			for (int i = 0; i < getNumSensores(); i++) {
				if(puntero->data.sampleSensores[i]){
					sampleSensoresCont[i]++;
				}
			}

			puntero = puntero->siguente;
		}

	}
}


Terminal::Nodo* Terminal::CrearNodo(Data data){
	Nodo* nodo = (Nodo*) malloc(sizeof(Nodo));
	nodo->data = data;
	nodo->siguente = NULL; //Apunta por defecto
	return nodo;
}

void Terminal::DestruirNodo(Nodo* nodo){
	free(nodo);
}

int Terminal::listaLongitud(Lista* lista){
	return lista->longitud;
}

int Terminal::listaVacia(Lista* lista){
	return lista->cabeza == NULL;
}

void Terminal::purgarNodosViejos(Lista* lista){

	if(lista->cabeza == NULL){
		return;
	}

	Nodo* puntero = lista->cabeza;

	while(puntero && (puntero->data.marcaTiempo + TIEMPO_VIDA_NODO) <= millis()){
		Nodo* eliminado = puntero;
		puntero = puntero->siguente; //Muevo el iterador una posicion
		lista->cabeza = puntero; //Actualizo la cabeza

		DestruirNodo(eliminado);
		lista->longitud--;
		nodosRevisados--; //Los descarto para equilibrar la lista

		Serial.println("INTER: NODO VIEJO BORRADO");

	}
}


void Terminal::RecorrerLista(Lista* lista){

	if(lista->cabeza == NULL){
		return;
	}

	Nodo* puntero = lista->cabeza;
	int contador = 0;

	while(puntero->siguente){
		printf("Nodo: %d valor: %d\n", contador, puntero->data.marcaTiempo);

		  for (int i = 0; i < 8; i++) {
		    Serial.print(puntero->data.sampleSensores[i]);
		    Serial.print(" ");
		  }
		  Serial.println();

		contador++;
		puntero = puntero->siguente; //Muevo el iterador una posicion
	}

	printf("Nodo: %d valor: %d\n", contador, puntero->data.marcaTiempo);
	for (int i = 0; i < 8; i++) {
		Serial.print(puntero->data.sampleSensores[i]);
		Serial.print(" ");
	}
	Serial.println();

}

void Terminal::InsertarPrincipio(Lista* lista, Data data){
	Nodo* nodo = CrearNodo(data);
	nodo->siguente = lista->cabeza; //Su siguente es el que estaba antes primero

	lista->cabeza = nodo; //Actualizo la lista
	lista->longitud++;
}


void Terminal::InsertarFinal(Lista* lista, Data data){
	Nodo* nodo = CrearNodo(data);

	if(lista->cabeza == NULL){
		lista->cabeza = nodo;
	}else{
		Nodo* puntero = lista->cabeza;

		while(puntero->siguente){
			puntero = puntero->siguente;
		}
		puntero->siguente = nodo;
	}
	lista->longitud++;
}


void Terminal::EliminarPrincipio(Lista* lista){
	if(lista->cabeza){
		Nodo* eliminado = lista->cabeza;
		lista->cabeza = lista->cabeza->siguente;
		DestruirNodo(eliminado);

		lista->longitud--;
	}
}

void Terminal::EliminarUltimo(Lista* lista){
	if(lista->cabeza){
		if(lista->cabeza->siguente){
			Nodo* puntero = lista->cabeza;
			while(puntero->siguente->siguente){
				puntero = puntero->siguente;
			}

			Nodo* eliminado = puntero->siguente;
			puntero->siguente = NULL;
			DestruirNodo(eliminado);

			lista->longitud--;
		}else {
			//Si solo hay un elemento
			Nodo* eliminado = lista->cabeza;
			DestruirNodo(eliminado);
			lista->cabeza = NULL;

			lista->longitud--;
		}
	}
}

String Terminal::serializarListaJson() {
    DynamicJsonDocument doc(1024*2); // Tamaño del buffer para el documento JSON

    // Crear un objeto JSON con la información de la lista
    JsonObject listaJson = doc.to<JsonObject>();
    //listaJson["longitud"] = listaTerminal.longitud;
      listaJson["longitud"] = min(listaTerminal.longitud, 10); // Limitar a 10 elementos

    JsonArray nodosJson = listaJson.createNestedArray("nodos");

    Nodo* nodoActual = listaTerminal.cabeza;
    byte contador = 0;

    while (nodoActual != nullptr && contador < 10) { // Limitar a 10 elementos
        JsonObject nodoJson = nodosJson.createNestedObject();
        nodoJson["marcaTiempo"] = nodoActual->data.marcaTiempo;

        JsonArray sensoresJson = nodoJson.createNestedArray("sampleSensores");
        for (int i = 0; i < 8; i++) {
            sensoresJson.add(nodoActual->data.sampleSensores[i]);
        }

        nodoActual = nodoActual->siguente;
        contador++;
    }

    Serial.print("Memoria: ");
    Serial.println(doc.memoryUsage());

    // Serializar el documento JSON en una cadena
    String jsonStr;
    serializeJson(doc, jsonStr);

    return jsonStr;

    Serial.println(jsonStr);

}


void Terminal::deserializarListJson(String jsonStr) {

	  Serial.println(jsonStr);

	if (jsonStr.isEmpty()) {
		Serial.println("No hay datos en la memoria NVS.");
		return;
	}

	DynamicJsonDocument doc(1024*2); // Tamaño del buffer para el documento JSON

	// Deserializar el documento JSON desde la cadena
	DeserializationError error = deserializeJson(doc, jsonStr);
	if (error) {
		Serial.println("Error al deserializar JSON en pila de terminal.");
		return;
	}

	Serial.print("Memoria: ");
	Serial.println(doc.memoryUsage());

	// Acceder a los datos deserializados y reconstruir la lista enlazada
	JsonObject listaJson = doc.as<JsonObject>();
	listaTerminal.longitud = listaJson["longitud"];

	JsonArray nodosJson = listaJson["nodos"];

	for (int i = nodosJson.size() - 1; i >= 0; i--){
		JsonObject nodoJson = nodosJson[i];
		Nodo* nuevoNodo = (Nodo*) malloc(sizeof(Nodo));
		nuevoNodo->siguente = NULL; //Apunta por defecto

		nuevoNodo->data.marcaTiempo = nodoJson["marcaTiempo"];

		JsonArray sensoresJson = nodoJson["sampleSensores"];
		for (int j = 0; j < 8; j++) {
			nuevoNodo->data.sampleSensores[j] = sensoresJson[j];
		}

		nuevoNodo->siguente = listaTerminal.cabeza;
		listaTerminal.cabeza = nuevoNodo;
	}
}
