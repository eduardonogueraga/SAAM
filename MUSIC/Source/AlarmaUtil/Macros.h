/*
 * Macros.h
 *
 *  Created on: 18 mar. 2023
 *      Author: isrev
 */

#ifndef SOURCE_MACROS_H_
#define SOURCE_MACROS_H_


//MACROS
//#define ALARMA_EN_MODO_DEBUG //Nunca definir en produccion
#define SENSOR_MG_CONECTADO //Definir cuando la alarma este conectada al sensor real
//#define WIFI_PUERTO_SERIE //Definir solo para debug


#define TIEMPO_OFF_TEST 0.1666
#define TIEMPO_ON_TEST 0.01
#define TIEMPO_REACTIVACION_TEST 0.1
#define TIEMPO_MODO_SENSIBLE_TEST 0.0166
#define TIEMPO_OFF_MODO_SENSIBLE 0.1666
#define TIEMPO_PRORROGA_GSM_TEST 0.05
#define TIEMPO_BOCINA_TEST 0.0333
#define TIEMPO_BOCINA_REACTIVACION_TEST 0.0666

//TECLADO
#define ROW_1 36
#define ROW_2 34
#define ROW_3 32
#define ROW_4 30
#define COL_1 38
#define COL_2 40
#define COL_3 42

//PANTALLA
#define CODEC_LCD 0x25 //  0x3F 0x27

//SENSORES
#define PIR_SENSOR_1 5
#define PIR_SENSOR_2 7
#define PIR_SENSOR_3 6
#define MG_SENSOR 	 4

//BOCINA
#define BOCINA_PIN 0

//WATCHDOG
#define WATCHDOG 3

//RS485
#define RS_CTL 13

//CONTROL RELES 12V
#define RELE_AUXILIAR 10

//SLEEPMODE
#define GSM_PIN 2
#define BT_PIN 12

//COMUNICACIONES
//#define GSM_TX 19
//#define GSM_RX 23

//#define RS_TX 18
//#define RS_RX 5

#define GSM_TX 18
#define GSM_RX 5

#define RS_TX 19
#define RS_RX 23


#define TEST_PIN_RS 13

//SD
#define REGISTRO_SS_PIN 53

//RESET
#define RESETEAR 9

//LED PUERTA
#define LED_COCHERA 8

//DIR I2C
#define MCP_ADDR 0x27  // mcp23017 address

//BATERIA E INTERRUPCIONES
#define SENSOR_BATERIA_RESPALDO 15 //Interrupcion por fallo en la bateria TODO No soportada en hardware
#define FALLO_BATERIA_PRINCIPAL 15 //Monitoriza el flujo de tension en el booster


//SISTEMA
#define TIEMPO_ALERT_LCD 6000
#define	NUMERO_ALERTAS 4
#define TOTAL_SENSORES 4 //EL numero de sensores activos
#define MAX_SIZE_JSON 2100 //Bytes maximos del modelo JSON
#define MAX_TOQUES_BOCINA_RESTANTES 15 //Intentos en modo inquieto
#define MAX_INTENTOS_REACTIVACION 3
#define MAX_PAQUETES_SAAS_EN_MEMORIA_SD 30
#define MAX_DESCARTE_PAQUETES_SAAS 10 //Paquetes descartados cuando se producza un HTTP 500
#define UMBRAL_FALLO_PAQUETE_SAAS 20
//COMUNICACION LINEA
enum infoTrazas{DESTINATARIO, AUTOR, METODO, METODO_DESC, ESTADO_SERVICIO, FOTOSENSOR};

typedef enum {
	MTH_DATA,
	MTH_REPLY,
	MTH_RETRY,
	MTH_RESET,
	MTH_FAIL
} metodosTrazas;

#define N_TERMINALES_LINEA 1

#define MAX_DATOS_TRAMA 6
#define MAX_DATOS_SUB_TRAMA 8
#define MAX_DATOS_CTL_LINEA 2
#define MAX_REINTENTOS_MASTER 2

enum infoMapeoSensores {
	L101_SENSOR,
	L102_SENSOR,
	L103_SENSOR,
	L104_SENSOR,
	L201_SENSOR,
	L202_SENSOR,
	L203_SENSOR,
	L204_SENSOR,
	C01_LINEA,
	C02_LINEA,
};

typedef struct  {
	byte dirSensor[4] = {MG_SENSOR, PIR_SENSOR_1, PIR_SENSOR_2, PIR_SENSOR_3};
	bool pirSensorAnt[4]= {true,true, true, true};
	byte sensorMG = 0;
	byte notificadoMG = 0; //Controla si se ha generado ya registro de MG
} InterStrikeCore;


//SAAS MODELO JSON
#define MAX_REINTENTOS_ENVIO_MODELO 3
#define MAX_REINTENTOS_ENVIO_NOTIFICACION 2

//SAA PILA TAREAS
#define MAX_REINTENTOS_REPROCESO_TAREA 2
#define MAX_NOTIFICACIONES_SYS_DIARIAS 50
#define MAX_NOTIFICACIONES_ALARM_DIARIAS 80
#define MAX_MODELO_JSON_DIARIOS 300


typedef enum {
	LIBRE,
	PROCESANDO
} PilaTareaEstado;


typedef enum {
	SILENCIO,
	TRAMA_KO,
	TRAMA_OK
} LecturasLinea;

typedef enum {
	LLAMAR_TERMINAL,
	ESCUCHAR_LINEA,
	SOLICITAR_REINTENTO,
	REINTENTAR
} EstadosTerminal;


typedef enum {
	TERMINAL_OK,
	DETECCION,
	DETECCION_FOTOSENIBLE,
	AVERIA,
	SABOTAJE,
	NO_REPLY,
	BAD_REPLY,
	BAD_COMM
} InterpretacionTerminal;



//DEFINICIONES DE FUNCIONES
void printSystemInfo();

void guardarFlagEE(const char* key, int value);
void guardarFlagEE(const char* key, uint8_t value);
//uint8_t leerFlagEE(const char* key); //Deprecated
int leerFlagEEInt(const char* key);
String leerCadenaEE(const char* key);
void guardarCadenaEE(const char* key, String* value);
void guardarEstadoInterrupcion();

template <typename T> void NVS_SaveData(const char* key, T value);
template <typename T> T NVS_RestoreData(const char* key);

String fixedLengthString(String& original, size_t fixedLength);

void encolarNotificacionSaas(byte tipo, const char* contenido);

void pantallaDeError(String mensaje);
//STRUCTS
struct listas_terminales_json{
	 char terminalCoreJson[2048];
};

typedef struct listas_terminales_json TerminalListJson;

struct datos_saltos_t{
	byte ID_SENSOR;
	byte ID_TERMINAL;
	byte INTENTOS_REACTIVACION = 0;
	TerminalListJson LISTADOS_TERMINALES;
};

typedef struct datos_saltos_t EE_DatosSalto;

struct configuracion_sistema_t {

	byte MODO_SENSIBLE = 1;
	byte MODULO_SD = 1;
	byte MODULO_RTC = 1;
	byte ENVIO_SAAS = 1;
	byte ENVIO_SAAS_NOTIFICACION = 1;
	byte ENVIO_FTP = 1;
	byte ESPERA_SAAS_MULTIPLICADOR = 0;
	byte ESCUCHAR_LINEA = 1;
	byte SENSORES_HABLITADOS[4] = {1,1,1,1};
	byte SMS_HISTORICO;
	char FECHA_SMS_HITORICO[30];

};

 typedef struct configuracion_sistema_t ConfigSystem;

 typedef struct  {
     int codigo = 0;
     String respuesta = "";
 } RespuestaHttp;

 typedef struct  {
     int error = 0;
     int numFicheros = 0;
     int saasLogid;
     String msg = "";
 } RespuestaFtp;

 typedef struct  {
     int intensidadSignal = 0;
     String proveedor = "";
 } ProveedorEstado;

 typedef struct  {
	 InterpretacionTerminal interpretacion;
	 String resumen = "";
	 byte idSensorDetonante;
	 byte idTerminal;
 } RespuestaTerminal;


 typedef struct  {
	 byte tipo;
	 char contenido[100];
 } NotificacionSaas;


 typedef struct  {
	 int  saasLogid = 0;
	 byte tipoLog;
	 char log[600];
 } RegistroLogTarea;


//ENUM Y TYPEDEFS
enum aliasConfigSistema{MD_SENS, MDL_SD, MDL_RTC};
enum ZonasSensor{MG,PIR_1,PIR_2,PIR_3};

typedef enum {
	ALARMA,
	MENU,
	ERROR
} ProcesoCentral;

typedef enum {
	ESTADO_REPOSO,
	ESTADO_GUARDIA,
	ESTADO_ALERTA,
	ESTADO_ENVIO,
	ESTADO_INQUIETO
} EstadosAlarma;

typedef enum {
	INFO_FALLO_BATERIA,
	INFO_SENSOR_PUERTA_OFF,
	INFO_RESET_AUTO,
	INFO_BLUETOOH
} Infos;

typedef enum {
	BT_OFF,
	BT_ON
} SLEEPMODE_BT;


typedef enum {
	TLF_1,
	TLF_2,
	COLGAR
} LLAMADAS_GSM;

typedef enum {
	ERR_FALLO_ALIMENTACION,
	ERR_FALLO_SENSOR
} CODIGO_ERROR;

typedef enum {
	GUARDAR_DATOS,
	COMPROBAR_DATOS,
	ENVIAR_AVISO,
	REALIZAR_LLAMADAS,
	ESPERAR_AYUDA
} EstadosError;


typedef enum {
	SMS_TIPO_SALTO,
	SMS_TIPO_INFO,
	SMS_TIPO_ERROR,
} TiposMensaje;


typedef enum {
	DIR_RAIZ,
	DIR_SISTEMA,
	DIR_HTTP,
	DIR_LOGS,
	DIR_JSON_REQUEST,
	DIR_HTTP_LOG,
	DIR_LOGS_BACKUP,
} RegistroDirectorios;

//LITERALES SAAS

typedef enum  {
  AVISO_ALARMA_MOVIMIENTO_PORCHE = 113,
  AVISO_ALARMA_MOVIMIENTO_COCHERA = 114,
  AVISO_ALARMA_MOVIMIENTO_ALMACEN = 115,
  AVISO_ALARMA_PUERTA_COCHERA_ABIERTA = 116,
  ALARMA_REACTIVADA_EXITO = 117,
  MOVIMIENTO_DETEC_SABOTAJE_ALIMENTACION = 118,
  FALLO_ALIMENTACION_PRINCIPAL = 119,
  BATERIA_EMERGENCIA_DESACTIVADA= 120
}	SAAS_LITERAL_NOTIFICACIONES;


typedef enum  {
	TLF_NUM_1 = 1,
	TLF_NUM_2 = 0,
	TLF_NUM_3= 2
}	SAAS_LITERAL_NOTIFICACIONES_TLF;


typedef enum  {
	ALARMA_INICIADA_LOG = 121,
	INTENTOS_SMS_DIARIOS_RECUPERADOS_LOG = 122,
	SENSOR_PUERTA_DESCONECTADO_LOG = 123,
	BATERIA_EMERGENCIA_ACTIVADA_LOG = 124,
	BATERIA_EMERGENCIA_DESACTIVADA_LOG = 125,
	INTENTOS_SMS_REALIZADOS_LOG = 126,
	INTENTOS_SMS_DIARIOS_ACABADOS_LOG = 127,
	ALARMA_ESTABLECIDA_MODO_DEFAULT_LOG = 128,
	ALARMA_ESTABLECIDA_MODO_PRUEBA_LOG = 129,
	LLAMANDO_A_MOVIL_LOG = 130,
	RESET_AUTOMATICO_LOG = 131,
	RESET_MANUAL_LOG = 132,
	FALLO_ALIMENTACION_LOG = 133,
	FALLO_SENSOR_LOG = 134,
	CARGADO_ESTADO_GUARDIA_PREVIO_LOG = 135,
	INTRUSISMO_RESTAURADO_LOG = 136,
	ALARMA_ACTIVADA_MANUALMENTE_LOG = 137,
	ALARMA_ACTIVADA_AUTOMATICAMENTE_LOG = 138,
	ALARMA_DESACTIVADA_MANUALMENTE_LOG = 139,
	ALARMA_DESACTIVADA_AUTOMATICAMENTE_LOG = 140,
	MODELO_ELIMINADO_EXCESO_REINTENTOS_LOG = 141,
	MODELO_ENVIADO_SD_LOG = 142,
	MODELO_ERROR_EN_ID_LOG = 143,
	MODELO_ERROR_ENVIO_LOG = 144,
	MODELO_ENVIADO_LOG = 145,
	INTENTOS_NOT_SYS_DIARIOS_RECUPERADOS_LOG  = 146,
	INTENTOS_NOT_ALR_DIARIOS_RECUPERADOS_LOG  = 147,
	INTENTOS_MODELO_JSON_DIARIOS_RECUPERADOS_LOG  = 148,
	FTP_ERROR_ABRIENDO_DIR_LOG  = 149,
	FTP_ERROR_ABRIENDO_CONEXION = 150,
	FTP_ERROR_TRANSFERENCIA = 151,
	FTP_ERROR_CERRANDO_CONEXION = 152,
	FTP_TRANFERENCIA_OK = 153,
	ERROR_GSM_SIN_COBERTURA = 154,
} SAAS_LITERAL_LOGS;


typedef enum  {
	P_MODO_NORMAL = 1,
	P_MODO_PHANTOM = 0,
	P_NO_RESTAURADO = 0,
	P_RESTAURADO = 1,
	P_ESTADO_ONLINE = 1,
	P_ESTADO_OFFLINE = 0,
}	SAAS_PARAMETROS_SALTO;

typedef enum  {
    ENVIAR_POR_POST,
    PROCESAR_RESPUESTA_ERROR,
    ACTUALIZAR_TOKEN,
    ABORTAR_ENVIO
} SAAS_GESTION_ENVIO;

typedef enum  {
    ENVIO_OK,
	ERROR_ENVIO,
	ERROR_ID
} SAAS_GESTION_ENVIO_R;

typedef enum  {
    ESPERA_ENVIO,
	ENVIO,
	ESPERA_REINTENTO
} SAAS_CRON_ENVIOS;

typedef enum  {
    PAQUETE,
	NOTIFICACION,
	FTP
} SAAS_TIPO_HTTP_REQUEST;


//MENU

typedef enum {
	SELECT,
	CONFIG,
	INFO,
	CHECK
} EstadosMenu;

typedef enum {
	CONFIG_SELECT,
	CONFIG_MODOS,
	CONFIG_MODULOS,
	CONFIG_SENSORES,
	CONFIG_SUBMENU,
	CONFIG_SAAS,
} MENU_CONFIG;


typedef enum {
	MOD_SELECT,
	MOD_FTP,
	MOD_SD,
	MOD_RTC
} MENU_CONFIG_MODULOS;

typedef enum {
	MODOS_SELECT,
	CH_MODO_TEST,
	CH_MODO_SENSIBLE
} MENU_CONFIG_MODOS;

typedef enum {
	SENSORES_SELECT,
	CH_MG,
	CH_PIR
} MENU_CONFIG_SENSORES;

typedef enum {
	SAAS_SELECT,
	CH_ACTIVACION,
	SAAS_CONF
} MENU_CONFIG_SAAS;

typedef enum {
	ACTIVAR_SELECT,
	ACTIVAR_MODELO,
	ACTIVAR_NOTIFICACIONES
} MENU_CONFIG_SAAS_ACTIVACION;

typedef enum {
	SAAS_SELECT_CONF,
	SYNC_ID_PAQUETE,
	SYNC_TOKEN_API,
	CH_CONF_TIEMPO
} MENU_CONFIG_SAAS_CONF;


typedef enum {
	INFO_SELECT,
	INFO_TIEMPO,
	INFO_DATOS,
	INFO_REGISTRO
} MENU_INFO;

typedef enum {
	TIME_MINUTOS,
	TIME_HORAS,
	TIME_DIAS,
	TIME_SEMANAS,
} MENU_INFO_TIME;

typedef enum {
	DATOS_ENTRADAS,
	DATOS_SMS_HISTORICO,
	DATOS_ERR_HISTORICO,
	DATOS_FECHA_RESET,
	DATOS_SMS_DIARIO,
	DATOS_PROVEEDOR_RED,
	DATOS_VERSION,
} MENU_INFO_DATOS;

typedef enum {
	REGISTROS_DESCARGAR,
	REGISTROS_BORRAR,
} MENU_INFO_REGISTROS;

//TEMPLATES GLOBALES
template <class T> uint8_t arrSum(byte arr[], byte tam){
	byte r = 0;

	for (int i = 0; i <tam; i++) {
		r += arr[i];
	}
	return r;
}


template <class T> void arrCopy(byte origen[], byte destino[], byte tam) {
	byte *p = origen;
	byte *q = destino;
	while (p != origen + tam)
		*q++ = *p++;
}

template <class T> void arrCopy(int origen[], int destino[], byte tam) {
	int *p = origen;
	int *q = destino;
	while (p != origen + tam)
		*q++ = *p++;
}

#endif /* SOURCE_MACROS_H_ */
