/*
 * Ftp.h
 *
 *  Created on: 19 nov. 2023
 *      Author: isrev
 */

#ifndef MUSIC_SOURCE_ALARMAUTIL_FTP_H_
#define MUSIC_SOURCE_ALARMAUTIL_FTP_H_

bool iniciarServicioFtp() {
	modem.sendAT(GF("+CFTPSSTART"));
	if (modem.waitResponse(10000L, GF(GSM_NL "+CFTPSSTART: 0")) != 1) { return false; }

	modem.sendAT(GF("+CFTPSSINGLEIP=1"));
	if (modem.waitResponse(5000L) != 1) { return false; }

	return true;
}

bool abrirSesionFtp() {
	char FTPConnectionCommand[150];
	sprintf(FTPConnectionCommand, "+CFTPSLOGIN=\"%s\",%i,\"%s\", \"%s\",0", serverUrl, 21, ftpUser, ftpPassword);

	modem.sendAT(GF(FTPConnectionCommand));
	if (modem.waitResponse(20000L, GF(GSM_NL "+CFTPSLOGIN: 0")) != 1) { return false; }
	return true;
}

bool cambiarDirectorioTrabajoFtp() {
	char FTPPwdCommand[150];
	sprintf(FTPPwdCommand, "+CFTPSCWD=\"%s\"", ftpWorkingDir);

	modem.sendAT(GF(FTPPwdCommand));
	if (modem.waitResponse(60000L, GF(GSM_NL "+CFTPSCWD: 0")) != 1) { return false; }
	return true;
}

String pwdFtp() {
	modem.sendAT(GF("+CFTPSPWD"));
	String res;
	if (modem.waitResponse(10000L, res) != 1) { return ""; }
	res.replace(GSM_NL "OK" GSM_NL, "");
	res.replace(GSM_NL, "");
	res.trim();
	return res;
}

bool enviarFicheroDesdeFSFtp() {
	modem.sendAT(GF("+CFTPSPUTFILE=\"toftp.txt\",3"));
	if (modem.waitResponse(60000L, GF(GSM_NL "+CFTPSPUTFILE: 0")) != 1) { return false; }
	return true;
}

bool enviarFicheroFtp(const char* nombreArchivo, int bytes, const char* buffer) {

	char FTPPutFileCommand[150];
	sprintf(FTPPutFileCommand, "+CFTPSPUT=\"%s\",%i", nombreArchivo, bytes);

	modem.sendAT(GF(FTPPutFileCommand));
	if (modem.waitResponse(10000L,GF(GSM_NL ">")) != 1) {
		Serial.println("No llega el caracter >");
		return false;
	}
	//delay(500);

	//Escribir el fichero desde serie
	modem.streamWrite(buffer);
	modem.stream.flush();

	if (modem.waitResponse(20000L, GF(GSM_NL "+CFTPSPUT: 0")) != 1) { return false; }
	return true;
}


bool cerrarSesionFtp() {
	modem.sendAT(GF("+CFTPSLOGOUT"));
	if (modem.waitResponse(10000L, GF(GSM_NL "+CFTPSLOGOUT: 0")) != 1) { return false; }
	return true;
}

bool detenerServicioFtp() {
	modem.sendAT(GF("+CFTPSSTOP"));
	if (modem.waitResponse(10000L, GF(GSM_NL "+CFTPSSTOP: 0")) != 1) { return false; }
	return true;
}

bool cerrarConexionFtp() {
    return cerrarSesionFtp() && detenerServicioFtp();
}

void testEnvioFtp(){

	if(!iniciarServicioFtp()){
		Serial.println("No se pudo iniciar el servicio FTP");
	}

	if(!abrirSesionFtp()){
		Serial.println("Error iniciando sesion FTP");
	}else {
		Serial.println("Login OK enviando fichero ...");
	}

	if(!cambiarDirectorioTrabajoFtp()){
		Serial.printf("Error moviendose a la ruta %s", ftpWorkingDir);
	}


	//if(enviarFicheroDesdeFSFtp()){
	//	Serial.println("Fichero enviado desde FS Ok");
	//}


	//Enviar fichero de prueba
	const char* nombreArchivo = "datos.txt";
	int tamanoArchivo = 50;
	const char* contenidoMensaje = "Este es el contenido del archivo de datos bytes123";

	if(enviarFicheroFtp(nombreArchivo, tamanoArchivo, contenidoMensaje)){
		Serial.println("Fichero enviado Ok");
	}else {
		Serial.println("KO enviando");
	}

	if(!cerrarConexionFtp()){
		Serial.println("Error cerrando sesion y servicio FTP");
		return;
	}else {
		Serial.println("Cerrando sesion y servicio FTP");
	}

}

#endif /* MUSIC_SOURCE_ALARMAUTIL_FTP_H_ */
