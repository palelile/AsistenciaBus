#ifndef CSV_LECTOR_H
#define CSV_LECTOR_H

#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QIODevice>
#include <QString>
#include <QStringList>
#include <QSqlTableModel>
#include <QSqlRecord>
#include "dblocal.h"

class csv_lector
{
private:
    QFile archivo_csv;
	QSqlTableModel *mi_modelo;
    dbLocal * mi_db;
public:
    csv_lector();
	bool leer_archivo_modelo(QString nombre_archivo);
    bool leer_archivo_db(QString nombre_archivo, dbLocal *base_datos);
	bool construir_cabecera_modelo(QString linea);
	bool leer_linea_modelo(QString linea);
	QString leer_linea_temporal_db(QString linea);
	QString leer_linea_curso_db(QString linea);
	QSqlTableModel * modelo();
};

#endif // CSV_LECTOR_H
