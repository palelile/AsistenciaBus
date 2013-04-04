#include "dbsync.h"

dbSync::dbSync()
{
}



void dbSync::sincronizarDB()
{
    if ( conectarDbLocal() && conectarDbRemota())
    {
        QStringList campos;

        campos << "rut" << "nombre" << "apellidoP" << "apellidoM" << "curso";
        SubirTabla("alumnos", "alumnos", campos);
        campos.clear();

        campos << "rut" << "fecha" << "hora";
        SubirTabla("asistenciamanana", "asistenciaBusManana", campos);
        campos.clear();

        campos << "rut" << "fecha" << "hora";
        SubirTabla("asistenciatarde", "asistenciaBusTarde", campos);
        campos.clear();

        campos << "rut" << "ruta";
        SubirTabla("autorizados", "autorizadosBus", campos);
        campos.clear();

        campos << "id" << "descripcion";
        SubirTabla("rutas", "rutasBus", campos);
        campos.clear();
    }
}

bool dbSync::conectarDbLocal()
{
    if ( db_local != NULL )
        return false;
    db_local = new QSqlDatabase();
    *db_local = QSqlDatabase::addDatabase("QSQLITE", "conexion_local");
    db_local->setDatabaseName("datos.db");
    return db_local->open();
}

bool dbSync::conectarDbRemota()
{
    if (db_remota == NULL)
        db_remota = new QSqlDatabase();
    *db_remota = QSqlDatabase::addDatabase("QMYSQL");
    db_remota->setHostName( r_hostname );
    db_remota->setDatabaseName( r_database );
    db_remota->setPort( r_port.toInt() );
    db_remota->setUserName( r_user );
    db_remota->setPassword( r_pass );
    return db_remota->open();
}

int dbSync::ultimoRegistro(QSqlDatabase *base_datos, QString tabla)
{
    if ( !db_local->isOpen() )
        return -1;
    QSqlQuery consulta( *db_local );
    if ( base_datos == db_local )
        consulta.prepare("SELECT ultimoregistrolocal  "
                         "FROM indices "
                         "WHERE tabla=:tabla");
    else
        consulta.prepare("SELECT ultimoregistroremoto "
                         "FROM indices "
                         "WHERE tabla=:tabla");
    consulta.bindValue(":tabla", tabla);
    if ( !consulta.exec() || !consulta.next() )
        return -1;
    return ( consulta.record().value(0).toInt() );
}

QVariantList * dbSync::TablaTemporal(QSqlDatabase *base_datos, QString tabla, QStringList campos, int ultimo_id)
{
    if ( !base_datos->isOpen() )
        return NULL;
    QVariantList * Listas = new QVariantList[ campos.count() ];
    QSqlQuery consulta_local(*base_datos);
    // Construyo la cadena de la consulta
    QString cadena_consulta = "SELECT ";
    for ( int contador = 0; contador < campos.count(); contador++ )
    {
        cadena_consulta.append(campos.at( contador ));
        cadena_consulta.append( ',' );
    }
    cadena_consulta[cadena_consulta.length() - 1] = ' ';
    cadena_consulta.append("FROM " + tabla + " WHERE id>");
    cadena_consulta.append(QString::number( ultimo_id ));
    consulta_local.prepare(cadena_consulta);
    if ( !consulta_local.exec() )
        return NULL;
    while ( consulta_local.next() )
    {
        for ( int i = 0; i < campos.count(); i++)
            if ( consulta_local.record().value(i).isNull() )
                Listas[i].append( QVariant(QVariant::String) );
            else
                Listas[i].append( consulta_local.record().value(i).toString() );
    }
    return Listas;
}

QList<QString> * dbSync::TablaTemporalCadenas(QSqlDatabase *base_datos, QString tabla, QStringList campos, int ultimo_id)
{
    if ( !base_datos->isOpen() )
        return NULL;
    QList<QString> * Listas = new QList<QString>;
    QSqlQuery consulta_local(*base_datos);
    // Construyo la cadena de la consulta
    QString cadena_consulta = "SELECT ";
    for ( int contador = 0; contador < campos.count(); contador++ )
    {
        cadena_consulta.append(campos.at( contador ));
        cadena_consulta.append( ',' );
    }
    cadena_consulta[cadena_consulta.length() - 1] = ' ';
    cadena_consulta.append("FROM " + tabla);
    cadena_consulta.append(" WHERE id>");
    cadena_consulta.append(QString::number( ultimo_id ));
    consulta_local.prepare(cadena_consulta);
    if ( !consulta_local.exec() )
        return NULL;
    QString *registro_temporal;
    while ( consulta_local.next() )
    {
        registro_temporal = new QString();
        for ( int i = 0; i < campos.count(); i++)
            if ( consulta_local.record().value(i).isNull() )
                registro_temporal->append( "," );
            else
                registro_temporal->append( consulta_local.record().value(i).toString() + ',' );
        (*registro_temporal).resize(registro_temporal->length() - 1);
        Listas->insert( Listas->size(), *registro_temporal);
    }
    return Listas;
}

bool dbSync::PersistirTablaTemporal(QSqlDatabase *base_datos, QString tabla, QStringList campos, QVariantList * listas)
{
    if ( !base_datos->isOpen() )
        return NULL;
    QSqlQuery consulta(*base_datos);
    // Construyo la cadena de la consulta
    QString cadena_consulta = "INSERT INTO " + tabla + " (";
    for ( int contador = 0; contador < campos.count(); contador++ )
        cadena_consulta.append(campos.at( contador ) + ",");
    cadena_consulta[cadena_consulta.length() - 1] = ')';
    cadena_consulta.append(" VALUES (");
    for ( int contador = 0; contador < campos.count(); contador++ )
        cadena_consulta.append(" ?,");
    cadena_consulta[cadena_consulta.length() - 1] = ')';
    consulta.prepare( cadena_consulta );
    for ( int i = 0; i < campos.count(); i++)
        consulta.addBindValue(listas[i]);
    return consulta.execBatch();
}

int dbSync::ultimoRegistroDB(QSqlDatabase *base_datos, QString tabla)
{
    QSqlQuery consulta(*base_datos);
    consulta.prepare("SELECT MAX(id) FROM " + tabla);
    if ( !consulta.exec() || !consulta.next() )
        return -1;
    return consulta.record().value(0).toInt();
}

bool dbSync::SubirTabla(QString tabla_local, QString tabla_remota, QStringList campos) {
    if ( !db_local->isOpen() || !db_remota->isOpen() )
        return false;
    // Recupero indice de los últimos registros sincronizados localmente y remotamente
    int ultimo_registro_local = ultimoRegistro(db_local, tabla_local);
    int ultimo_registro_remoto = ultimoRegistro(db_remota, tabla_local);
    // Tablas temporales con los registros que no han sido sincronizados
    QVariantList * tablaTemporalLocal = TablaTemporal(db_local, tabla_local, campos, ultimo_registro_local);
    QVariantList * tablaTemporalRemoto = TablaTemporal(db_remota, tabla_remota, campos, ultimo_registro_remoto);
    // Se hace permanente los registros en las bases de datos respectivas
    borrarRegistroEliminadoDBRemota(db_local, db_remota, tabla_local, tabla_remota, campos);
    borrarRegistroEliminadoDBRemota(db_remota, db_local, tabla_remota, tabla_local, campos);
    PersistirTablaTemporal(db_remota, tabla_remota, campos, tablaTemporalLocal);
    PersistirTablaTemporal(db_local, tabla_local, campos, tablaTemporalRemoto);
    ultimo_registro_local = ultimoRegistroDB( db_local, tabla_local );
    ultimo_registro_remoto = ultimoRegistroDB( db_remota, tabla_remota);
    return guardarUltimoRegistro(tabla_local, ultimo_registro_local, ultimo_registro_remoto);
}

bool dbSync::borrarRegistroEliminadoDBRemota(QSqlDatabase *db_1, QSqlDatabase *db_2, QString tabla1, QString tabla2, QStringList campos){
    if ( !db_1->isOpen() || !db_2->isOpen() )
        return false;
    int ultimo_registro_1 = ultimoRegistro(db_1, tabla1);
    int ultimo_registro_2 = ultimoRegistro(db_2, tabla1);
    QList<QString> * tablaTemporal1 = TablaTemporalCadenas(db_1, tabla1, campos, ultimo_registro_1);
    QList<QString> * tablaTemporal2 = TablaTemporalCadenas(db_2, tabla2, campos, ultimo_registro_2);

    for (int i=0; i < tablaTemporal1->count(); i++ )
        if ( !tablaTemporal2->contains( tablaTemporal1->value( i ) ))
            borrarRegistro( db_1, tabla1, campos, tablaTemporal1->value( i ) );
    for (int i=0; i < tablaTemporal2->count(); i++ )
        if ( !tablaTemporal1->contains( tablaTemporal2->value( i ) ))
            borrarRegistro( db_2, tabla2, campos, tablaTemporal2->value( i ) );
    return true;
}

bool dbSync::borrarRegistro(QSqlDatabase *db, QString tabla, QStringList campos, QString registro)
{
    if ( !db->isOpen() )
        return NULL;
    QSqlQuery consulta(*db);
    QString cadena_consulta = "DELETE FROM ";
    cadena_consulta.append(tabla + " WHERE ");
    for ( int contador = 0; contador < campos.count(); contador++ )
        cadena_consulta.append(campos.at( contador ) + "=? AND ");
    for (int i = 0; i < 4; i++)
        cadena_consulta[cadena_consulta.length() - i -1] = ' ';
    consulta.prepare( cadena_consulta );
    for ( int i = 0; i < campos.count(); i++)
        consulta.addBindValue(registro.at( i ));
    return consulta.exec();
}

bool dbSync::guardarUltimoRegistro(QString tabla, int ultimo_registro_local, int ultimo_registro_remoto)
{
    QSqlQuery consulta_local(*db_local);
    consulta_local.prepare("DELETE FROM indices WHERE tabla=:tabla");
    consulta_local.bindValue(":tabla", tabla);
    consulta_local.exec();
    consulta_local.finish();
    consulta_local.prepare("INSERT INTO indices (tabla,ultimoregistrolocal,ultimoregistroremoto) VALUES (:tabla,:ultimoregistrolocal,:registroremoto)");
    consulta_local.bindValue(":tabla", tabla);
    consulta_local.bindValue(":ultimoregistrolocal", ultimo_registro_local);
    consulta_local.bindValue(":ultimoregistroremoto", ultimo_registro_remoto);
    return consulta_local.exec();
}


// Carga las opciones del programa
void dbSync::cargarOpciones(){
    QSettings opciones(QSettings::IniFormat, QSettings::UserScope, "Ashtaroth", "AsistenciaBus");
    opciones.beginGroup("DB");
    r_hostname = opciones.value("Direccion").toString();
    r_port = opciones.value("Puerto").toString();
    r_database = opciones.value("DB").toString();
    r_user = opciones.value("Usuario").toString();
    r_pass = opciones.value("Password").toString();
    opciones.endGroup();
}
