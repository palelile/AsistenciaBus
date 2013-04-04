#ifndef DBSYNC_H
#define DBSYNC_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

#include <QVariantList>
#include <QVariant>

#include <QStringList>
#include <QString>

#include <QSettings>

class dbSync
{
public:
    void sincronizarDB();
    dbSync();
    bool conectarDbLocal();
    bool conectarDbRemota();
    int ultimoRegistro(QSqlDatabase *base_datos, QString tabla);
    QVariantList * TablaTemporal(QSqlDatabase *base_datos, QString tabla, QStringList campos, int ultimo_id);
    QList<QString> * TablaTemporalCadenas(QSqlDatabase *base_datos, QString tabla, QStringList campos, int ultimo_id);
    bool PersistirTablaTemporal(QSqlDatabase *base_datos, QString tabla, QStringList campos, QVariantList * listas);
    int ultimoRegistroDB(QSqlDatabase *base_datos, QString tabla);
    bool SubirTabla(QString tabla_local, QString tabla_remota, QStringList campos);
    bool borrarRegistroEliminadoDBRemota(QSqlDatabase *db_1, QSqlDatabase *db_2, QString tabla1, QString tabla2, QStringList campos);
    bool borrarRegistro(QSqlDatabase *db, QString tabla, QStringList campos, QString registro);
    bool guardarUltimoRegistro(QString tabla, int ultimo_registro_local, int ultimo_registro_remoto);
private:
    QSqlDatabase * db_local;
    QSqlDatabase * db_remota;
    QString r_hostname;
    QString r_database;
    QString r_port;
    QString r_user;
    QString r_pass;
    // Cargar datos
    void cargarOpciones();
};

#endif // DBSYNC_H
