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


void testTaskNodos(){

	static int idTest;
	DatosTarea datosNodo;

	datosNodo.tipoPeticion = NOTIFICACION;
	datosNodo.notificacion.tipo = idTest;
	strcpy(datosNodo.notificacion.contenido, "Deteccion en sitio");

	InsertarFinal(&listaTareas, datosNodo, 0,0);
	RecorrerPilaTareas(&listaTareas);

	idTest++;
}


void testTaskNodos2(){

	static int idTest;
	DatosTarea datosNodo;

	datosNodo.tipoPeticion = PAQUETE;
	datosNodo.notificacion.tipo = idTest;
	strcpy(datosNodo.notificacion.contenido, "No deberias pasar esto");

	InsertarFinal(&listaTareas, datosNodo, 0,0);
	RecorrerPilaTareas(&listaTareas);

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
