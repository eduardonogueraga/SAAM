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
	DatosTarea data; //Datos que van en el taskNodo
	struct TaskNodo* siguente;
}TaskNodo;


typedef	struct PilaTareas {
	TaskNodo* cabeza;
	int longitud;
} PilaTareas;


PilaTareas listaTareas;


//-----------------------------------------------------------

TaskNodo* CrearTaskNodo(DatosTarea data){
	TaskNodo* taskNodo = (TaskNodo*) malloc(sizeof(TaskNodo));
	taskNodo->data = data;
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
		printf("TaskNodo: %d Tipo peticion: %d Tipo de notificacion: %d Contenido noti: %s \n",
				contador,
				puntero->data.tipoPeticion,
				puntero->data.notificacion.tipo,
				puntero->data.notificacion.contenido
				);

		contador++;
		puntero = puntero->siguente; //Muevo el iterador una posicion
	}
}


void InsertarPrincipio(PilaTareas* lista, DatosTarea data){
	TaskNodo* taskNodo = CrearTaskNodo(data);
	taskNodo->siguente = lista->cabeza; //Su siguente es el que estaba antes primero

	lista->cabeza = taskNodo; //Actualizo la lista
	lista->longitud++;
}


void InsertarFinal(PilaTareas* lista, DatosTarea data){
	TaskNodo* taskNodo = CrearTaskNodo(data);

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

//----------------------------------------------------------


void testTaskNodos(){

	DatosTarea datosNodo;
	datosNodo.tipoPeticion = PAQUETE;

	datosNodo.notificacion.tipo = 1;
	strcpy(datosNodo.notificacion.contenido, "Valor para contenido2");

	InsertarFinal(&listaTareas, datosNodo);
	RecorrerPilaTareas(&listaTareas);
}


void testTaskNodosDelete(){
	EliminarPrincipio(&listaTareas);
	RecorrerPilaTareas(&listaTareas);
}


#endif /* SOURCE_PILATAREAS_H_ */
