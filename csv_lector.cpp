#include "csv_lector.h"

csv_lector::csv_lector()
{
	//mi_db = new db();
	mi_modelo = new QSqlTableModel();
}

/* Lee un archivo CVS considerando la primera linea como cabecera, asume que
 * el archivo tiene el formato correcto
 */
bool csv_lector::leer_archivo_modelo(QString nombre_archivo)
{
	QFile file(nombre_archivo);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	if ( !construir_cabecera_modelo( QString::fromLatin1(file.readLine()) ))
		return false;
	while ( !file.atEnd() )
		if ( !leer_linea_modelo( QString::fromUtf8(file.readLine()) ) )
			break;
	file.close();
	return true;
}

bool csv_lector::leer_archivo_db(QString nombre_archivo, dbLocal *base_datos)
{
	mi_db = base_datos;
	QFile file(nombre_archivo);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QTextStream linea(&file);

	linea.setCodec(QTextCodec::codecForName("ISO-8859-15"));
	linea.readLine(); // Asumo que tiene cabecera
	QString datos ="";
	int i = 1;
	while ( !linea.atEnd() )
	{
		datos.append( leer_linea_temporal_db( linea.readLine() ) );
		i++;
		if ( i%300 == 0 || linea.atEnd() )
		{
			mi_db->personas_agregar_muchas( datos );
			datos.clear();
		}
	}
	mi_db->consolidar_temporal();
	file.close();
	return true;
}

bool csv_lector::construir_cabecera_modelo(QString linea)
{
	QStringList campos = linea.split(";");
	if ( campos.isEmpty() )
		return false;
	for ( int i = 0; i < campos.count(); i++ )
		mi_modelo->setHeaderData(i, Qt::Horizontal, campos.value(i, ""));
	return true;
}

bool csv_lector::leer_linea_modelo(QString linea)
{
	QStringList campos = linea.split(";");
	QSqlRecord fila;
	if ( campos.isEmpty() )
		return false;
	for ( int i = 0; i < campos.count(); i++ )
		fila.setValue( i ,campos.value(i) );
	return mi_modelo->insertRecord( -1, fila);
}

QString csv_lector::leer_linea_temporal_db(QString linea)
{
	QStringList campos = linea.split(";");
	if ( campos.isEmpty() || campos.count() != 25)
		return "";
	QString salida = "('";
	for ( int i = 0; i < 25; i++ )
		salida.append(campos.value(i) + "','");
	salida.truncate( salida.length() - 3);
	salida.append("'),");
	return salida;
}

QString csv_lector::leer_linea_curso_db(QString linea)
{
	QStringList campos = linea.split(";");
	if ( campos.isEmpty() || campos.count() != 25)
		return "";
	QString salida = "";
	salida.append("('" + campos.value(0) + "','" + campos.value(2) + "','" + campos.value(3)  + "','" + campos.value(5) + "','" + campos.value(4) + "'),");
	return salida;
}

QSqlTableModel * csv_lector::modelo()
{
	return mi_modelo;
}
