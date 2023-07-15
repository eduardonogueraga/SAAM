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

Terminal::Terminal(char* nombreTerminal, byte numFotoSensor, byte numLineasCtl, byte numSensores) {

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
}

byte Terminal::getDatosFotosensor() const {
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

void Terminal::setDatosFotosensor(byte datosFotosensor) {
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



void Terminal::guardarDatosTerminal(){

	Data data1;
	srand(time(NULL));
	data1.value = rand();

	this->InsertarFinal(&this->listaTerminal, data1);
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

void Terminal::RecorrerLista(Lista* lista){

	if(lista->cabeza == NULL){
		return;
	}

	Nodo* puntero = lista->cabeza;
	int contador = 0;

	while(puntero->siguente){
		printf("Nodo: %d valor: %d\n", contador, puntero->data.value);
		contador++;
		puntero = puntero->siguente; //Muevo el iterador una posicion
	}

	printf("Nodo: %d valor: %d\n", contador, puntero->data.value);

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
