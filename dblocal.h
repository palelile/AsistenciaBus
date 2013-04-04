#ifndef DBLOCAL_H
#define DBLOCAL_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

#include <QVariant>
#include <QDate>
#include <QTime>
#include <QList>

class dbLocal
{
public:
    QSqlDatabase * db_local;
    dbLocal();
    bool validarRut(QString rut);
    QString infoRut(QString rut);
    bool validarAutorizacion(QString rut);
    bool almacenarRegistro(QString rut, int opcion);
    bool verificarRutaEnDb(int id, QString descripcion);
    bool agregarRutaEnDb(int id, QString descripcion);
    int autorizarRut( QString rut, int ruta );
    int desAutorizarRut( QString rut );
    QList<QString*> generarReporte(int *nAlumnos, int tipo, int *fin, QDate fechaInicio, QDate fechaFin);
    bool personas_agregar_muchas(QString datos);
    void consolidar_temporal();
private:
    bool inicializar();
    QString cap(const QString &str);
};

#endif // DBLOCAL_H
