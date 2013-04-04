#ifndef BUS_H
#define BUS_H

#include <QMainWindow>
#include <QTextCodec>

#include <QDateTime>
#include <QString>
#include <QTextStream>
#include <QTextDocument>
#include <QPrinter>
#include <QPrintDialog>
#include <QFileDialog>
#include <QSettings>
#include <QFileDialog>

#include "dblocal.h"
#include "csv_lector.h"
#include "dbsync.h"

namespace Ui {
class Bus;
}

class Bus : public QMainWindow
{
	Q_OBJECT

public:
	explicit Bus(QWidget *parent = 0);
	~Bus();

private slots:
    // Intercambio de áreas de trabajo
	void cambioDePagina(int pagina);

    // Página agregar asistencia
	bool agregarAsistencia();

    // Página agregar rutas
	void agregarRuta();

    // Página autorizar usuarios
	void autorizar();
	void desautorizar();

    // Página reportes
	void poblarTabla();
	void GuardarReporte();
	void ImprimirReporte();

    // Importar y sincronizar datos
    void sincronizarDBs();
    void importar_Sige();

private:
	Ui::Bus *ui;
    dbLocal *db_local;

    // Funciones administrativas

    // Página agregar asistencia
	void limpiarAsistencia();
    void mensajeAsistencia(int opcion, QString alumno);

    // Página autorizar usuarios
    void mensajeErrorAutorizar(int opcion, QString alumno);
    void limpiarAutorizacion();
    void autorizarRut( QString rut, int ruta );
    void limpiarDesAutorizacion();

    // Permanencia de opciones
	void cargarOpciones();
	void guardarOpciones();

    // Funciones de asistencia
    bool esViajeManana();
};

#endif // BUS_H
