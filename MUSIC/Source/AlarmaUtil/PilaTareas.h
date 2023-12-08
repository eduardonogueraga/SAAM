/*
 * PilaTareas.h
 *
 *  Created on: 13 ago. 2023
 *      Author: Eduardo
 *
 * Las funciones no se comparten con las definidas en
 * terminal para evitar colisiones entre tareas
 *
 */

#ifndef SOURCE_PILATAREAS_H_
#define SOURCE_PILATAREAS_H_

struct DatosTarea {
	SAAS_TIPO_HTTP_REQUEST tipoPeticion;
	NotificacionSaas notificacion;
};

typedef	struct TaskNodo {
	int posicion;
	byte reintentos;
	unsigned long tiempoEspera;
	DatosTarea data; //Datos que van en el taskNodo
	struct TaskNodo* siguente;
}TaskNodo;


typedef	struct PilaTareas {
	TaskNodo* cabeza;
	TaskNodo* tareaEnCurso; //Referencia a la tarea en curso
	int longitud;
} PilaTareas;


PilaTareas listaTareas;


//-----------------------------------------------------------

TaskNodo* CrearTaskNodo(DatosTarea data, byte reintentos = 0, unsigned long tiempoEspera = 0L){
	TaskNodo* taskNodo = (TaskNodo*) malloc(sizeof(TaskNodo));
	taskNodo->data = data;
	taskNodo->reintentos = reintentos;
	taskNodo->tiempoEspera = (tiempoEspera) ? tiempoEspera: 0L;
	taskNodo->siguente = NULL; //Apunta por defecto
	return taskNodo;
}

void DestruirTaskNodo(TaskNodo* taskNodo){
	free(taskNodo);
}

int listaLongitud(PilaTareas* lista){
	return lista->longitud;
}

int listaVacia(PilaTareas* lista){
	return lista->cabeza == NULL;
}


void RecorrerPilaTareas(PilaTareas* lista){

	if(lista->cabeza == NULL){
		return;
	}

	TaskNodo* puntero = lista->cabeza;
	int contador = 0;

	while(puntero){

		printf("TaskNodo: %d Tipo peticion: %d Reintentos: %d Not Type: %d Contenido not: %s  Tiempo: %d\n",
				contador,
				puntero->data.tipoPeticion,
				puntero->reintentos,
				puntero->data.notificacion.tipo,
				puntero->data.notificacion.contenido,
				(puntero->tiempoEspera > millis()) ? (millis() - puntero->tiempoEspera): 0
				);
		contador++;
		puntero = puntero->siguente; //Muevo el iterador una posicion
	}
}


void InsertarPrincipio(PilaTareas* lista, DatosTarea data, byte reintentos = 0){
	TaskNodo* taskNodo = CrearTaskNodo(data,reintentos);
	taskNodo->siguente = lista->cabeza; //Su siguente es el que estaba antes primero

	lista->cabeza = taskNodo; //Actualizo la lista
	lista->longitud++;
}


void InsertarFinal(PilaTareas* lista, DatosTarea data, byte reintentos = 0, unsigned long tiempoEspera = 0L){
	TaskNodo* taskNodo = CrearTaskNodo(data,reintentos, tiempoEspera);

	if(lista->cabeza == NULL){
		lista->cabeza = taskNodo;
	}else{
		TaskNodo* puntero = lista->cabeza;

		while(puntero->siguente){
			puntero = puntero->siguente;
		}
		puntero->siguente = taskNodo;
	}
	lista->longitud++;
}


void EliminarPrincipio(PilaTareas* lista){
	if(lista->cabeza){
		TaskNodo* eliminado = lista->cabeza;
		lista->cabeza = lista->cabeza->siguente;
		DestruirTaskNodo(eliminado);

		lista->longitud--;
	}
}

void EliminarUltimo(PilaTareas* lista){
	if(lista->cabeza){
		if(lista->cabeza->siguente){
			TaskNodo* puntero = lista->cabeza;
			while(puntero->siguente->siguente){
				puntero = puntero->siguente;
			}

			TaskNodo* eliminado = puntero->siguente;
			puntero->siguente = NULL;
			DestruirTaskNodo(eliminado);

			lista->longitud--;
		}else {
			//Si solo hay un elemento
			TaskNodo* eliminado = lista->cabeza;
			DestruirTaskNodo(eliminado);
			lista->cabeza = NULL;

			lista->longitud--;
		}
	}
}

TaskNodo* recuperarPrimerElemento(PilaTareas* lista){
	if(lista->cabeza){
		TaskNodo* tarea = lista->cabeza;
		return tarea;
	}else {
		return NULL;
	}
}


TaskNodo* recuperarTareaProcesable(PilaTareas* lista) {
    TaskNodo* puntero = lista->cabeza;
    int contador = 0;
    while (puntero != NULL) {

        if (puntero->tiempoEspera < millis()) {
        	puntero->posicion = contador; //Posicion n desde 0 a Tam

        	//Marco esta tarea como tarea en curso
        	lista->tareaEnCurso = puntero;

            return puntero;
        }
        puntero = puntero->siguente;
        contador++;
    }

    return NULL;
}

void EliminarTareaEnPosicion(PilaTareas* lista, int posicion) {
    if (posicion < 0 || posicion >= lista->longitud) {
        // La posición está fuera de los límites de la lista
        return;
    }

    if (posicion == 0) {
        EliminarPrincipio(lista);
        return;
    }

    TaskNodo* puntero = lista->cabeza;

    // Avanza hasta el nodo anterior al que se va a eliminar
    for (int i = 0; i < posicion - 1; i++) {
        puntero = puntero->siguente;
    }

    TaskNodo* nodoAEliminar = puntero->siguente;
    puntero->siguente = nodoAEliminar->siguente;

    DestruirTaskNodo(nodoAEliminar);
    lista->longitud--;
}

TaskNodo* tareaEnCurso(PilaTareas* lista){
	return lista->tareaEnCurso;
}

void finalizarTareaEnCurso(PilaTareas* lista){
	 lista->tareaEnCurso = NULL;
}

//----------------------------------------------------------
// Creacion y encolado de tareas


void encolarEnvioModeloSaas(){
	//Se guarda en la cola el envio periodico al saas
	DatosTarea datosNodo;
	datosNodo.tipoPeticion = PAQUETE;

	//Comprobamos si quedan envios
	if(leerFlagEEInt("N_MOD_SEND") >= MAX_MODELO_JSON_DIARIOS){
		registro.registrarLogSistema("SUPERADO MAXIMO ENVIOS MODELO DIARIOS");
		return;
	}

	InsertarFinal(&listaTareas, datosNodo);

	guardarFlagEE("N_MOD_SEND", (leerFlagEEInt("N_MOD_SEND")+1));
}


void crearTareaEnvioModeloSaas(){
	xTaskCreatePinnedToCore(
			tareaSaas,
			"tareaSaas",
			(1024*15), //Buffer
			NULL, //Param
			1, //Prioridad
			&envioServidorSaas, //Task
			0);
}

byte enviarEnvioModeloSaas(){
	byte executionResult;


#ifdef ALARMA_EN_MODO_DEBUG
	if(modem.waitForNetwork(2000, true)){
#else
	if(modem.waitForNetwork(2000, true)){ //@develop NO NEGAR EN PROD
#endif
		Serial.println(F("Hay cobertura se procede al envio"));
		executionResult = eventosJson.enviarInformeSaas();
	}else {
		Serial.println(F("No hay cobertura se aborta el envio"));
		executionResult = 0;
		//Refresco el modulo
		refrescarModuloGSM();
	}

	return executionResult;
}

void encolarNotificacionSaas(byte tipo, const char* contenido){

	if(!configSystem.ENVIO_SAAS_NOTIFICACION)
		return;

	//Se guarda en la cola la notificacion para ser procesada
	DatosTarea datosNodo;

	datosNodo.tipoPeticion = NOTIFICACION;
	datosNodo.notificacion.tipo = tipo;
	strcpy(datosNodo.notificacion.contenido, contenido);

	//Comprobamos si quedan envios
	if(leerFlagEEInt("N_SYS_SEND") >= MAX_NOTIFICACIONES_SYS_DIARIAS && datosNodo.notificacion.tipo == 0){
		registro.registrarLogSistema("SUPERADO MAXIMO NOTIFICACIONES_SYS_DIARIAS");
		return;
	}

	if(leerFlagEEInt("N_ALR_SEND") >= MAX_NOTIFICACIONES_ALARM_DIARIAS && datosNodo.notificacion.tipo == 1){
		registro.registrarLogSistema("SUPERADO MAXIMO NOTIFICACIONES_ALR_DIARIAS");
		return;
	}

	InsertarFinal(&listaTareas, datosNodo);

	if(datosNodo.notificacion.tipo == 1){
		guardarFlagEE("N_ALR_SEND", (leerFlagEEInt("N_ALR_SEND")+1));
	}else {
		guardarFlagEE("N_SYS_SEND", (leerFlagEEInt("N_SYS_SEND")+1));
	}
}

void crearTareaNotificacionSaas(byte tipo, const char* contenido){
	//Se definen los datos de la notificacion y se crea una tarea en segundo plano
	 datosNotificacionSaas.tipo = tipo;
	 strcpy(datosNotificacionSaas.contenido, contenido);

		xTaskCreatePinnedToCore(
				tareaNotificacionSaas,
				"tareaNotificacionSaas",
				(1024*10), //Buffer
				&datosNotificacionSaas, //Param
				1, //Prioridad
				&envioNotificacionSaas, //Task
				0);
}

byte enviarNotificacionesSaas(byte tipo, const char* contenido){

	byte resultado;

#ifdef ALARMA_EN_MODO_DEBUG
	if(modem.waitForNetwork(2000, true)){
#else
	if(modem.waitForNetwork(2000, true)){ //@develop NO NEGAR EN PROD
#endif
		Serial.println(F("Hay cobertura se procede al envio"));
		resultado = eventosJson.enviarNotificacionSaas(tipo, contenido);
	}else {
		Serial.println(F("No hay cobertura se aborta el intento"));
		resultado = 0;
		//Refresco el modulo
		refrescarModuloGSM();
	}

	if (resultado == 1) {
		Serial.println(F("Notificacion enviada exitosamente."));
	} else {
		Serial.println(F("Fallo al enviar notificacion."));
	}

	return resultado;
}


void encolarNotifiacionIntrusismo(){
	char contenidoCola[200];
	char resumenTerminal[50];
	respuestaTerminal.resumen.toCharArray(resumenTerminal, 50);

	sprintf(contenidoCola, "\%s, %s:%s",
			(respuestaTerminal.interpretacion == DETECCION)? "Intrusismo":
			(respuestaTerminal.interpretacion == DETECCION_FOTOSENIBLE)? "Luz detectada":
			(respuestaTerminal.interpretacion == AVERIA)? "Averia":
			(respuestaTerminal.interpretacion == SABOTAJE)? "Sabotaje": "Intrusismo",
			 literalesZonas[respuestaTerminal.idTerminal][respuestaTerminal.idSensorDetonante],
			 resumenTerminal
	);

	encolarNotificacionSaas(1, contenidoCola);
}

void encolarEnvioFtpSaas(){
	//Se guarda en la cola el envio ftp diario
	DatosTarea datosNodo;
	datosNodo.tipoPeticion = FTP;

	InsertarFinal(&listaTareas, datosNodo);
}

void crearTareaEnvioFtpSaas(){
	xTaskCreatePinnedToCore(
			tareaFtpSaas,
			"tareaFtp",
			(1024*10), //Buffer
			NULL, //Param
			1, //Prioridad
			&envioFtpSaas, //Task
			0);
}

byte enviarEnvioFtpSaas(){
	byte executionResult;
	RespuestaFtp respuesta;
	RegistroLogTarea reg;
	reg.tipoLog = 0; //systema

#ifdef ALARMA_EN_MODO_DEBUG
	if(modem.waitForNetwork(2000, true)){
#else
		if(modem.waitForNetwork(2000, true)){ //@develop NO NEGAR EN PROD
#endif
			Serial.println(F("Hay cobertura se procede al envio"));
			respuesta = registro.envioRegistrosFTP();
			executionResult = !respuesta.error; //Si no hay problem le envio un 1 como OK

			if(!respuesta.error){
				snprintf(reg.log, sizeof(reg.log), "Envio FTP realizado con exito (%i)", respuesta.numFicheros);
			}else {
				Serial.println(F("Error en el envio reiniciando GSM"));
				refrescarModuloGSM();
				snprintf(reg.log, sizeof(reg.log), "Error en envio FTP: %s", respuesta.msg.c_str());
			}

			reg.saasLogid = respuesta.saasLogid;

			xQueueSend(colaRegistros, &reg, 0);

		}else {
			Serial.println(F("No hay cobertura se aborta el envio"));
			executionResult = 0;
			refrescarModuloGSM();
		}

		return executionResult;
}


void gestionarPilaDeTareas(){
	//Compruebo si se ha habilitado la gestion de tareas
	if(!accesoGestorPila)
		return;

	if(!checkearMargenTiempo(tiempoReinicioPila))
		return;

	//Compruebo si la lista esta vacia
	if(listaTareas.cabeza == NULL){
		//Serial.println("Pila vacia");
		return;
	}

	//Se controla que no pueda haber un desbordamiento
	if (listaLongitud(&listaTareas) > MAX_NODOS_EN_EJECUCION) {
		Serial.println("Overload en pila de tareas");
		EliminarPrincipio(&listaTareas);
	}

	TaskNodo* tarea;
	TaskHandle_t manejador;
	unsigned long tiempoReintento;

	//Comprobamos si hay alguna tarea en curso
	tarea = tareaEnCurso(&listaTareas);

	if(tarea == NULL){
		//Serial.println("No hay tareas en curso buscamos una nueva");
		//Comprobar si alguno de los elementos de la lista esta ya listo para procesarse
		tarea = recuperarTareaProcesable(&listaTareas);

		if(tarea == NULL){
			//Serial.println("No hay nada en la pila listo para procesar");
			//Si ninguna de las tareas esta lista volvemos
			return;
		}
	}


	if(tarea->data.tipoPeticion == NOTIFICACION){
		manejador = envioNotificacionSaas;
	}else if(tarea->data.tipoPeticion == PAQUETE) {
		manejador = envioServidorSaas;
	}else if(tarea->data.tipoPeticion == FTP) {
		manejador = envioFtpSaas;
	}else {
		manejador = NULL;
	}

	//Definimos el tiempo de los reintentos
	tiempoReintento = TIEMPO_ESPERA_REINTENTO_TAREA;

	if(estadoAlarma == ESTADO_ALERTA){
		tiempoReintento = (TIEMPO_ESPERA_REINTENTO_TAREA*0.5);
	}


	//Si hay una tarea en cola compruebo si esta en ejecucion
	if(manejador == NULL){
		Serial.print("Tarea no creada, Reintentos: ");
		Serial.println(tarea->reintentos);

		//Creo la tarea conveniente en funcion del ipo de peticion
		if(tarea->data.tipoPeticion == NOTIFICACION){
			//Creo la tarea para notificaciones
			Serial.println("Creando tarea notificacion");
			crearTareaNotificacionSaas(tarea->data.notificacion.tipo, tarea->data.notificacion.contenido);
		}

		if(tarea->data.tipoPeticion == PAQUETE){
			//Creo la tarea para el envio de paquetes
			Serial.println("Creando tarea envio modelo json");
			crearTareaEnvioModeloSaas();
		}

		if(tarea->data.tipoPeticion == FTP){
			//Creo la tarea para el envio de paquetes
			Serial.println("Creando tarea envio ftp log");
			crearTareaEnvioFtpSaas();
		}

		//Definimos los tiempos de ejecucion para las tareas
		if(tarea->data.tipoPeticion != FTP){
			if(estadoAlarma == ESTADO_ALERTA || estadoAlarma == ESTADO_ENVIO){
				setMargenTiempo(tiempoTareaEnEjecucion,TIEMPO_MAX_TAREA);
			}else {
				setMargenTiempo(tiempoTareaEnEjecucion,(TIEMPO_MAX_TAREA*2));
			}

		}else {
			setMargenTiempo(tiempoTareaEnEjecucion,(TIEMPO_MAX_TAREA_FTP));
		}

		estadoPila = PROCESANDO;

	}else {
		//La tarea existe compruebo su estado
		eTaskState estadoTarea = eTaskGetState(manejador);

		//Serial.print("Estado de la tarea eTaskState: ");
		//Serial.println(estadoTarea);

		if(estadoTarea == eSuspended){
			//Suspendemos cuando termina OK
			Serial.println("Tarea finalizada");
			//Libero la tarea para que ella sola se diriga hacia su irremediable final
			vTaskResume(manejador);

			//Compruebo el flag global de la tarea http
			byte resultadoTarea;

			if(tarea->data.tipoPeticion == NOTIFICACION){
				resultadoTarea = resultadoEnvioNotificacionSaas;
			}

			if(tarea->data.tipoPeticion == PAQUETE) {
				resultadoTarea = resultadoEnvioServidorSaas;
			}

			if(tarea->data.tipoPeticion == FTP) {
				resultadoTarea = resultadoEnvioFtpSaas;
			}

			if(resultadoTarea){
				//Elimino el nodo
				Serial.println("Resultado OK borro la tarea");
				EliminarTareaEnPosicion(&listaTareas,tarea->posicion);
			}else {

				if(tarea->reintentos == MAX_REINTENTOS_REPROCESO_TAREA){
					Serial.println("Tarea ko supera los reintnetos");
					EliminarTareaEnPosicion(&listaTareas,tarea->posicion);
				}else {
					//Si no ha sido exitosa movemos al final con timeout
					Serial.println("Tarea ko reintento con timeout");
					tarea->reintentos++;
					EliminarTareaEnPosicion(&listaTareas,tarea->posicion);
					InsertarFinal(&listaTareas, tarea->data, tarea->reintentos, (millis()+tiempoReintento));
				}

			}

			estadoPila = LIBRE;
			finalizarTareaEnCurso(&listaTareas);

		}else {
			//Dejamos trabajar a la tarea y controlamos el tiempo

			if(checkearMargenTiempo(tiempoTareaEnEjecucion) || paradaDeEmergenciaPila == 1){
				Serial.println(!paradaDeEmergenciaPila ? "Se supera el tiempo de ejecucion" : "Se detiene la tarea solicitud de parada");

				//Cerramos la tareas directamente
				//eRunning 0 , eReady 1 , eBlocked 2, eSuspended 3, etc.
				if(tarea->data.tipoPeticion == NOTIFICACION){
					vTaskSuspend(envioNotificacionSaas);
					//Elimino la tarea
					vTaskDelete(envioNotificacionSaas);
					envioNotificacionSaas = NULL;

				}else if(tarea->data.tipoPeticion == PAQUETE) {
					vTaskSuspend(envioServidorSaas);
					//Elimino la tarea
					vTaskDelete(envioServidorSaas);
					envioServidorSaas = NULL;
				}else if(tarea->data.tipoPeticion == FTP) {
					vTaskSuspend(envioFtpSaas);
					//Elimino la tarea
					vTaskDelete(envioFtpSaas);
					envioFtpSaas = NULL;
				}

				if(tarea->reintentos == MAX_REINTENTOS_REPROCESO_TAREA || paradaDeEmergenciaPila == 1){
					//Elimino la tarea
					Serial.println(!paradaDeEmergenciaPila ? "Eliminando tarea por exceso de intentos" : "Eliminando tarea por solicitud de parada");

					EliminarTareaEnPosicion(&listaTareas, tarea->posicion);
					if(paradaDeEmergenciaPila ==1){paradaDeEmergenciaPila = 0;}

				}else {
					tarea->reintentos++;
					//Muevo la tarea al final
					Serial.println("Tarea pospuesta");
					EliminarTareaEnPosicion(&listaTareas,tarea->posicion);
					InsertarFinal(&listaTareas, tarea->data, tarea->reintentos, (millis()+tiempoReintento));
				}

				estadoPila = LIBRE;
				finalizarTareaEnCurso(&listaTareas);
			}

		}
	}
}

void rehabilitarEjecucionPila(){
	accesoGestorPila = 1; //Acceso a pila con tiempo de espera
	setMargenTiempo(tiempoReinicioPila,(TIEMPO_REINCIO_PILA));
}

void detenerEjecucionPila(){
	accesoGestorPila = 0;

	if(envioNotificacionSaas != NULL){
		vTaskDelete(envioNotificacionSaas);
		envioNotificacionSaas = NULL;
	}

	if(envioServidorSaas != NULL){
		vTaskDelete(envioServidorSaas);
		envioServidorSaas = NULL;
	}
	if(envioFtpSaas != NULL){
		vTaskDelete(envioFtpSaas);
		envioFtpSaas = NULL;
	}
}

void detenerEjecucionTaskFTP(){
	if(envioFtpSaas != NULL){
		Serial.println("El envio FTP se detuvo debido a una emergencia");
		paradaDeEmergenciaPila = 1;
		accesoAlmacenamientoSD = 1; //Vuelvo a abrir el paso de logs
		refrescarModuloGSM();
	}
}

//----------------------------------------------------------

void testTaskNodos(){

	static int idTest;
	DatosTarea datosNodo;

	datosNodo.tipoPeticion = NOTIFICACION;
	datosNodo.notificacion.tipo = 1;
	strcpy(datosNodo.notificacion.contenido, "Deteccion en un lugar indeterminado");

	//Comprobamos si quedan envios
	if(leerFlagEEInt("N_SYS_SEND") >= MAX_NOTIFICACIONES_SYS_DIARIAS && datosNodo.notificacion.tipo == 0){
		registro.registrarLogSistema("SUPERADO MAXIMO NOTIFICACIONES_SYS_DIARIAS");
		return;
	}

	if(leerFlagEEInt("N_ALR_SEND") >= MAX_NOTIFICACIONES_ALARM_DIARIAS && datosNodo.notificacion.tipo == 1){
		registro.registrarLogSistema("SUPERADO MAXIMO NOTIFICACIONES_ALR_DIARIAS");
		return;
	}


	InsertarFinal(&listaTareas, datosNodo, 0,0);
	RecorrerPilaTareas(&listaTareas);

	if(datosNodo.notificacion.tipo == 1){
		guardarFlagEE("N_ALR_SEND", (leerFlagEEInt("N_ALR_SEND")+1));
		Serial.println("aumento");
	}else {
		guardarFlagEE("N_SYS_SEND", (leerFlagEEInt("N_SYS_SEND")+1));
	}

	idTest++;
}


void testTaskNodos2(){

	static int idTest;
	DatosTarea datosNodo;

	datosNodo.tipoPeticion = PAQUETE;
	datosNodo.notificacion.tipo = idTest;
	strcpy(datosNodo.notificacion.contenido, "No deberias pasar esto");


	//Comprobamos si quedan envios
	if(leerFlagEEInt("N_MOD_SEND") >= MAX_MODELO_JSON_DIARIOS){
		registro.registrarLogSistema("SUPERADO MAXIMO ENVIOS MODELO DIARIOS");
		return;
	}

	InsertarFinal(&listaTareas, datosNodo, 0,0);
	RecorrerPilaTareas(&listaTareas);


	guardarFlagEE("N_MOD_SEND", (leerFlagEEInt("N_MOD_SEND")+1));
	idTest++;

}


void testTaskNodosTimeout(){

	TaskNodo* tarea = recuperarPrimerElemento(&listaTareas);
	EliminarPrincipio(&listaTareas);
	InsertarFinal(&listaTareas, tarea->data, tarea->reintentos, (millis()+ 35000));

	RecorrerPilaTareas(&listaTareas);
}


void testTaskNodosRecuperarProcesable(){
	TaskNodo* tarea = recuperarTareaProcesable(&listaTareas);
	if(tarea != NULL){
		Serial.print("ID:");
		Serial.println(tarea->posicion);

		Serial.println("Moviendo al final");
		EliminarTareaEnPosicion(&listaTareas,tarea->posicion);
		InsertarFinal(&listaTareas, tarea->data, tarea->reintentos);

		RecorrerPilaTareas(&listaTareas);


	}else {
		Serial.print("No hay tareas procesables en este momento");
	}

}

void testTaskNodosDelete(){
    EliminarPrincipio(&listaTareas);
	RecorrerPilaTareas(&listaTareas);
}

void testTaskNodosRecorrer(){
	RecorrerPilaTareas(&listaTareas);
}


void testTaskNodosMover(){
	TaskNodo* tarea = recuperarPrimerElemento(&listaTareas);
	tarea->reintentos++;

	EliminarPrincipio(&listaTareas);
	InsertarFinal(&listaTareas, tarea->data, tarea->reintentos);

    RecorrerPilaTareas(&listaTareas);
}


#endif /* SOURCE_PILATAREAS_H_ */
