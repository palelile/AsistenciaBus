#include "dblocal.h"

dbLocal::dbLocal()
{
    db_local = NULL;
    if ( !inicializar() )
        exit(1);
}

bool dbLocal::inicializar()
{
    if ( db_local != NULL )
        return false;
    db_local = new QSqlDatabase();
    *db_local = QSqlDatabase::addDatabase("QSQLITE", "conexion_local");
    db_local->setDatabaseName("datos.db");
    return db_local->open();
}


// Verifica que el rut se encuentre en la DB
bool dbLocal::validarRut(QString rut)
{
    if ( !db_local->isOpen() )
        return false;
    QSqlQuery query(*db_local);
    query.prepare("SELECT id "
                  "FROM alumnos "
                  "WHERE rut=:rut");
    query.bindValue(":rut", rut);
    return ( query.exec() && query.next() );
}


// Información de persona con rut
QString dbLocal::infoRut(QString rut)
{
    QString alumno = "";
    if ( !db_local->isOpen() )
        return alumno;
    QSqlQuery query(*db_local);
    query.prepare("SELECT nombre,apellidoP,apellidoM,curso "
                  "FROM alumnos "
                  "WHERE rut=:rut");
    query.bindValue(":rut", rut);
    if ( query.exec() && query.next() )
    {
        alumno += cap( query.record().value(0).toString() );
        alumno += cap( query.record().value(1).toString() );
        alumno += cap( query.record().value(2).toString() );
        alumno += cap( query.record().value(3).toString() );
    }
    return alumno;
}


// Verifica que el rut se encuentre autorizado
bool dbLocal::validarAutorizacion(QString rut)
{
    if ( !db_local->isOpen() )
        return false;
    QSqlQuery query(*db_local);
    query.prepare("SELECT id "
                  "FROM autorizados "
                  "WHERE rut=:rut");
    query.bindValue(":rut", rut);
    return ( query.exec() && query.next() );
}



// Guarda la asistencia: opción 1, primer viaje del día,
// opción 2 para el segundo viaje.
bool dbLocal::almacenarRegistro(QString rut, int opcion)
{
    if ( !db_local->isOpen() )
        return false;
    QSqlQuery query(*db_local);
    QDate fecha = QDate::currentDate();
    QTime hora = QTime::currentTime();
    switch ( opcion )
    {
    case 1:
        query.prepare("INSERT INTO asistenciamanana "
                      "(rut,fecha,hora) "
                      "VALUES "
                      "(:rut,:fecha,:hora)");
        break;
    case 2:
        query.prepare("INSERT INTO asistenciatarde "
                      "(rut,fecha,hora) "
                      "VALUES "
                      "(:rut,:fecha,:hora)");
        break;
    }
    query.bindValue(":rut", rut);
    query.bindValue(":fecha", fecha);
    query.bindValue(":hora", hora);
    return ( query.exec() && query.next() );
}


// Verifica que la ruta dada no se encuentre en la base de datos
bool dbLocal::verificarRutaEnDb(int id, QString descripcion)
{
    if ( !db_local->isOpen() )
        return false;
    QSqlQuery query(*db_local);
    query.prepare("SELECT * "
                  "FROM rutas "
                  "WHERE id=:id AND descripcion=:descripcion");
    query.bindValue(":id", id);
    query.bindValue(":descripcion", descripcion);
    return ( query.exec() && query.next() );
}


// Agrega una nueva ruta a la DB
bool dbLocal::agregarRutaEnDb(int id, QString descripcion)
{
    if ( !db_local->isOpen() )
        return false;
    QSqlQuery query(*db_local);
    query.prepare("INSERT INTO rutas "
                  "(id,descripcion) "
                  "VALUES "
                  "(:id,:descripcion)");
    query.bindValue(":id", id);
    query.bindValue(":descripcion", descripcion);
    if ( query.exec() )
        return verificarRutaEnDb(id, descripcion);
    return false;
}


// Permite a un RUT usar una ruta
int dbLocal::autorizarRut( QString rut, int ruta )
{
    if ( !db_local->isOpen() )
        return -1;
    if ( !validarRut( rut ) )
        return 1;
    QSqlQuery query(*db_local);
    if ( !validarAutorizacion( rut ) )
        query.prepare("INSERT INTO autorizados (rut,ruta) VALUES (:rut,:ruta)");
    else {
        query.prepare("UPDATE autorizados SET ruta=:ruta WHERE rut=:rut");
        return 2;
    }
    query.bindValue(":rut", rut);
    query.bindValue(":ruta", ruta);
    query.exec();
}


// Niega a un RUT usar una ruta
int dbLocal::desAutorizarRut( QString rut )
{
    if ( !db_local->isOpen() )
        return -1;
    if ( !validarRut( rut ) )
        return 1;
    if ( !validarAutorizacion( rut ) )
        return 3;
    QSqlQuery query(*db_local);
    query.prepare("DELETE FROM autorizados "
                  "WHERE rut=:rut");
    query.bindValue(":rut", rut);
    return query.exec();
}


QList<QString*> dbLocal::generarReporte(int *nAlumnos, int tipo, int *fin, QDate fechaInicio, QDate fechaFin)
{
    QList<QString*> alumnos;
    QString *alumno;
    *fin = 0;
    if ( !db_local->isOpen() )
        return alumnos;
    QSqlQuery query(*db_local);
    switch ( tipo )
    {
    case 0:     // Asistencia diaria por ruta mañana
        query.prepare( "SELECT asistenciamanana.fecha,rutas.descripcion,count() "
                       "FROM rutas,asistenciamanana,autorizados "
                       "WHERE rutas.id=autorizados.ruta "
                       "AND asistenciamanana.rut=autorizados.rut "
                       "AND asistenciamanana.fecha>=:fechaInicio "
                       "AND asistenciamanana.fecha<=:fechaFin "
                       "GROUP BY asistenciamanana.fecha AND rutas.id");
        query.bindValue(":fechaInicio", fechaInicio);
        query.bindValue(":fechaFin", fechaFin);
        *fin = 3;
        break;
    case 1:     // Asistencia diaria por ruta tarde
        query.prepare( "SELECT asistenciatarde.fecha,rutas.descripcion,count() "
                       "FROM rutas,asistenciatarde,autorizados "
                       "WHERE rutas.id=autorizados.ruta "
                       "AND asistenciatarde.rut=autorizados.rut "
                       "AND asistenciatarde.fecha>=:fechaInicio "
                       "AND asistenciatarde.fecha<=:fechaFin "
                       "GROUP BY asistenciatarde.fecha AND rutas.id");
        query.bindValue(":fechaInicio", fechaInicio);
        query.bindValue(":fechaFin", fechaFin);
        *fin = 3;
        break;
    case 2:     // Asistencia por alumno mañana
        query.prepare( "SELECT alumnos.curso,alumnos.nombre,alumnos.apellidoP,alumnos.apellidoM,count() "
                       "FROM asistenciamanana,alumnos "
                       "WHERE alumnos.rut=asistenciamanana.rut "
                       "AND asistenciamanana.fecha>=:fechaInicio "
                       "AND asistenciamanana.fecha<=:fechaFin "
                       "GROUP BY asistenciamanana.rut;");
        query.bindValue(":fechaInicio", fechaInicio);
        query.bindValue(":fechaFin", fechaFin);
        *fin = 5;
        break;
    case 3:     // Asistencia por alumno tarde
        query.prepare( "SELECT alumnos.curso,alumnos.nombre,alumnos.apellidoP,alumnos.apellidoM,count() "
                       "FROM asistenciatarde,alumnos "
                       "WHERE alumnos.rut=asistenciatarde.rut "
                       "AND asistenciatarde.fecha>=:fechaInicio "
                       "AND asistenciatarde.fecha<=:fechaFin "
                       "GROUP BY asistenciatarde.rut;");
        query.bindValue(":fechaInicio", fechaInicio);
        query.bindValue(":fechaFin", fechaFin);
        *fin = 5;
        break;
    case 4: // Inasistencia por ruta, indicando curso y alumno (mañana)
        query.prepare("SELECT rutas.descripcion AS Ruta,curso AS Curso,nombre||' '||apellidoP||' '||apellidoM AS Alumno "
                      "FROM alumnos,autorizados,rutas "
                      "WHERE alumnos.rut IN "
                      " (SELECT rut FROM autorizados WHERE rut NOT IN "
                      "  (SELECT rut FROM asistenciamanana WHERE fecha=:fecha) "
                      ") "
                      "AND alumnos.rut=autorizados.rut "
                      "AND autorizados.ruta=rutas.id "
                      "ORDER BY Ruta,Curso,apellidoP");
        query.bindValue(":fecha", fechaFin);
        *fin = 3;
        break;
    case 5: // Inasistencia por ruta, indicando curso y alumno (tarde)
        query.prepare("SELECT rutas.descripcion AS Ruta,curso AS Curso,nombre||' '||apellidoP||' '||apellidoM AS Alumno "
                      "FROM alumnos,autorizados,rutas WHERE alumnos.rut IN "
                      "(SELECT rut FROM autorizados WHERE rut NOT IN "
                      " (SELECT rut FROM asistenciatarde WHERE fecha=:fecha) "
                      " ) "
                      "AND alumnos.rut=autorizados.rut "
                      "AND autorizados.ruta=rutas.id "
                      "ORDER BY Ruta,Curso,apellidoP");
        query.bindValue(":fecha", fechaFin);
        *fin = 3;
        break;
    default:
        break;
    }
    query.exec();

    while ( query.next() )
        (*nAlumnos)++;

    if ( *nAlumnos > 0 ){
        query.first();
        do {
            alumno = new QString[*fin];
            for ( int i = 0; i < *fin; i++ )
                alumno[i] = cap( query.record().value(i).toString() );
            alumnos.append(alumno);
        } while ( query.next() );
        query.finish();
        return alumnos;
    }
    return alumnos;
}

/*******************************************************************************
 * Funciones de asistencia
 ******************************************************************************/

// Convierte un texto a "Tipo Titulo"
QString dbLocal::cap(const QString &str)
{
    QString tmp = str;
    tmp = tmp.toLower();
    tmp[0] = str[0].toUpper();
    for ( int i = 1; i < tmp.length(); i++ )
        if ( tmp[i-1] == ' ' )
            tmp[i] = str[i].toUpper();
    tmp += " ";
    return tmp;
}


/*******************************************************************************
 * Funciones de importación TXT
 ******************************************************************************/

bool dbLocal::personas_agregar_muchas(QString datos)
{
    QSqlQuery consulta(*db_local);
    datos.truncate( datos.length() - 1 );
    consulta.prepare( "INSERT OR IGNORE INTO temporal "
                      "VALUES " + datos + ";");
    return consulta.exec();
}

void dbLocal::consolidar_temporal()
{
    QSqlQuery consulta(*db_local);
    consulta.exec("INSERT OR IGNORE INTO 'alumnos' "
                  "(rut,nombre,apellidoP,apellidoM,curso)"
                  "SELECT run_alumno,nombre_alumno,ape_paterno_alumno,ape_materno_alumno,"
                  "glosa_grado||' '||letra_curso "
                  "FROM temporal ORDER BY run_alumno");
}
