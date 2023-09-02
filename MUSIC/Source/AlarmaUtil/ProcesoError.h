/*
 * ProcesoError.h
 *
 *  Created on: 2 sept. 2023
 *      Author: Eduardo
 *
 *      Nodos de la maquina de estados de error
 *
 */

#ifndef SOURCE_ALARMAUTIL_PROCESOERROR_H_
#define SOURCE_ALARMAUTIL_PROCESOERROR_H_

void setEstadoErrorComprobarDatos(){
		Serial.println(F("Guardando datos "));
		estadoError = COMPROBAR_DATOS;
		guardarEstadoAlerta();

		estadoAlarma = ESTADO_REPOSO;

		pararBocina();
		estadoLlamada = TLF_1;

		guardarFlagEE("ESTADO_GUARDIA", 0);
		guardarFlagEE("ESTADO_ALERTA", 0);
		guardarFlagEE("F_RESTAURADO", 0);
		guardarFlagEE("F_REACTIVACION", 0);

		registro.registrarLogSistema("ALARMA DESACTIVADA AUTOMATICAMENTE");
		eventosJson.guardarEntrada();
		eventosJson.guardarLog(ALARMA_DESACTIVADA_AUTOMATICAMENTE_LOG);

	}

	void setEstadoErrorEnviarAviso(){

		Serial.println(F("Comprobando datos "));
		estadoError = ENVIAR_AVISO;

		//Cerramos la pila de tareas y terminamos la ejecucion si quedase alguna tarea ejecutandose
		detenerEjecucionPila();
		//Cerramos el acceso a los terminales
		ACCESO_LISTAS = 0;

		sleepModeGSM = GSM_ON;
		setMargenTiempo(tiempoMargen,TIEMPO_CARGA_GSM);

		if(!modem.waitForNetwork(1000, true)){
			Serial.println(F("Modulo sin red refrescando"));
			refrescarModuloGSM();
		}

	}

	void setEstadoErrorRealizarLlamadas(){
		estadoError = REALIZAR_LLAMADAS;
		setMargenTiempo(tiempoMargen,240000);
	}

	void setEstadoErrorEsperarAyuda(){
		Serial.println(F("Esperar ayuda"));

		//Liberadas los SMS y llamadas encolamos peticiones
		encolarNotificacionSaas(0, "Interrupcion por fallo en la alimentacion");
		encolarEnvioModeloSaas(); //Encolamos otro modelo tras el envio de la alarma

		rehabilitarEjecucionPila();

		estadoError = ESPERAR_AYUDA;
	}

	void procesoError(){

		switch(estadoError){

		case GUARDAR_DATOS:
			setEstadoErrorComprobarDatos();
			break;

		case COMPROBAR_DATOS:

			if(!isLcdInfo())
			pantalla.lcdLoadView(&pantalla, &Pantalla::errorEmergencia);

			setEstadoErrorEnviarAviso();
			break;

		case ENVIAR_AVISO:

			if(!isLcdInfo())
				pantalla.lcdLoadView(&pantalla, &Pantalla::errorEmergencia);

			if(checkearMargenTiempo(tiempoMargen)){
				mensaje.mensajeError();
				setEstadoErrorRealizarLlamadas();
			}
			desactivarEstadoDeError();

			break;

		case REALIZAR_LLAMADAS:

			if(!isLcdInfo())
				pantalla.lcdLoadView(&pantalla, &Pantalla::errorEmergencia);

			realizarLlamadas();

			if(checkearMargenTiempo(tiempoMargen)){
				setEstadoErrorEsperarAyuda();
				guardarFlagEE("LLAMADA_EMERGEN", 1);
			}

			desactivarEstadoDeError();
			break;
		case ESPERAR_AYUDA:

			if(!isLcdInfo())
				pantalla.lcdLoadView(&pantalla, &Pantalla::errorEmergencia);

			desactivarEstadoDeError();
			break;
		}
	}

#endif /* SOURCE_ALARMAUTIL_PROCESOERROR_H_ */
