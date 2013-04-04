#include "bus.h"
#include "ui_bus.h"

Bus::Bus(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::Bus)
{

	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
	ui->setupUi(this);
	connect(ui->toolBox, SIGNAL(currentChanged(int)), this, SLOT(cambioDePagina(int)));
    connect(ui->bot_imp_txt, SIGNAL(pressed()), this, SLOT(importar_Sige()) );
	cargarOpciones();
	this->showFullScreen();
	ui->toolBox->setCurrentIndex(0);
	ui->asistenciaRut->setFocus();
    db_local = new dbLocal();
}

Bus::~Bus()
{
	guardarOpciones();
	delete ui;
}


/*******************************************************************************
 * Funciones administrativas
 ******************************************************************************/

// Intercambio de áreas de trabajo
void Bus::cambioDePagina(int pagina){
    switch (pagina) {
    case 0: // Agregar asistencia
        ui->asistenciaRut->setFocus();
        break;
    case 1:
        ui->rutasRutaNueva->setFocus();
        break;
    case 2:
        ui->autorizarIngresarRut->setFocus();
        break;
    case 3:
    {
        ui->reporteFin->setDate(QDate::currentDate());
        ui->reporteInicio->setDate(QDate::currentDate());
    }
        break;
    case 4:
    {
        ui->progressBar->setVisible(false);
        ui->syncButton->setFocus();
    }
        break;
    default:
        break;
    }
}




/*******************************************************************************
 * Página 0: Agregar asistencia
 ******************************************************************************/

// Valida si el rut es candidato para la asistencia, si es así, lo ingresa a la
// DB.
bool Bus::agregarAsistencia() {
    QString rut = ui->asistenciaRut->text();
    limpiarAsistencia();
    bool estado = false;

    if ( ! db_local->validarRut( rut ) ) {
        mensajeAsistencia(1, "");   // Rut no existe en registro
        return estado;
    }
    if ( ! db_local->validarAutorizacion( rut ) ) {
        mensajeAsistencia(2, db_local->infoRut( rut ));   // Alumno no autorizado a viajar
        return estado;
    }
    if ( esViajeManana( ) ){
        estado = db_local->almacenarRegistro(rut, 1);
        mensajeAsistencia(3, db_local->infoRut( rut ));
    }
    else {
        estado = db_local->almacenarRegistro(rut, 2);
        mensajeAsistencia(4, db_local->infoRut( rut ));
    }
    return estado;
}

// Limpia la ventana de asistencia.
void Bus::limpiarAsistencia() {
    ui->asistenciaRut->setText("");
    ui->asistenciaRut->setFocus();
}

// Mensaje de información de error por ventana
void Bus::mensajeAsistencia(int opcion, QString alumno) {
    QString mensaje = "";
    switch (opcion) {
    case 1:
        mensaje = "RUT no pertenece a ningún alumno";
        break;
    case 2:
        mensaje = alumno + "<br />no ha sido autorizado a viajar.";
        break;
    case 3:
        mensaje = alumno + "<br />ha iniciado el día.";
        break;
    case 4:
        mensaje = alumno + "<br />ha finalizado el día.";
        break;
    default:
        mensaje = "Error sin definir.";
        break;
    }
    ui->asistenciaInfo->setText(mensaje);
    ui->asistenciaInfo->setStyleSheet("font-size : 26px; background-color : #BC3A00; color : #FFFFFF;");
}



/*******************************************************************************
 * Página 1: Agregar ruta
 ******************************************************************************/

// Agrega una nueva ruta al listado de rutas
void Bus::agregarRuta() {
    if ( ui->rutasRutaNueva->text().compare("") != 0 ){
        db_local->agregarRutaEnDb(ui->rutasRuta->count(), ui->rutasRutaNueva->text());
        ui->rutasRuta->addItem( ui->rutasRutaNueva->text() );
        ui->autorizarRuta->addItem( ui->rutasRutaNueva->text() );
    }
    ui->rutasRutaNueva->setText(NULL);
}


/*******************************************************************************
 * Página 2: Autorizar usuarios en ruta
 ******************************************************************************/

void Bus::autorizar()
{
    int mensaje = db_local->autorizarRut( ui->autorizarIngresarRut->text(), ui->autorizarRuta->currentIndex() );
    mensajeErrorAutorizar(mensaje, db_local->infoRut( ui->autorizarIngresarRut->text() ));
    limpiarAutorizacion();
}

void Bus::desautorizar()
{
    int mensaje = db_local->desAutorizarRut( ui->autorizarRemoverRut->text() );
    mensajeErrorAutorizar(mensaje, db_local->infoRut( ui->autorizarRemoverRut->text() ));
    limpiarDesAutorizacion();
}

// Mensaje de información de error por ventana
void Bus::mensajeErrorAutorizar(int opcion, QString alumno) {
    QString mensaje = "";
    switch (opcion) {
    case 1:
        mensaje = "RUT no pertenece a ningún alumno";
        break;
    case 2:
        mensaje = alumno + ",<br /> ya autorizado a viajar, se ha asignado la nueva ruta.";
        break;
    case 3:
        mensaje = alumno + ",<br /> no posee autorización previa.";
        break;
    default:
        mensaje = "Error sin definir.";
        break;
    }
    ui->autorizarInfo->setText(mensaje);
    ui->autorizarInfo->setStyleSheet("font-size : 26px; background-color : #BC3A00; color : #FFFFFF;");
}

void Bus::limpiarAutorizacion(){
    ui->autorizarIngresarRut->setText(NULL);
    ui->autorizarIngresarRut->setFocus();
}

void Bus::limpiarDesAutorizacion(){
    ui->autorizarRemoverRut->setText(NULL);
    ui->autorizarRemoverRut->setFocus();
}



/*******************************************************************************
 * Página 3: Reportes
 ******************************************************************************/
void Bus::poblarTabla()
{
    int indice = ui->reporteOpcion->currentIndex();
    QStringList labels;
    int nAlumnos = 0;
    int columnas = 0;

    QList<QString*> alumnos = db_local->generarReporte( &nAlumnos, indice, &columnas, ui->reporteInicio->date(), ui->reporteFin->date() );

    switch (indice){
    case 0: // Asistencia por ruta «mañana»
        labels << "Fecha" << "Ruta" << "Asistencia mañana";
        break;
    case 1: // Asistencia por ruta «tarde»
        labels << "Fecha" << "Ruta" << "Asistencia tarde";
        break;
    case 2: // Asistencia por alumno «mañana»
        labels << "Curso" << "Nombre" << "Apellido Paterno" << "Apellido Materno" << "Asistencia mañana";
        break;
    case 3: // Asistencia por alumno «tarde»
        labels << "Curso" << "Nombre" << "Apellido Paterno" << "Apellido Materno" << "Asistencia tarde";
        break;
    case 4: // Inasistencia del dia «mañana»
        labels << "Ruta" << "Curso" << "Alumnos";
        break;
    case 5: // Inasistencia del dia «tarde»
        labels << "Ruta" << "Curso" << "Alumnos";
        break;
    }

    ui->tablaReporte->setColumnCount(columnas);
    ui->tablaReporte->setHorizontalHeaderLabels(labels);
    ui->tablaReporte->verticalHeader()->hide();
    ui->tablaReporte->setShowGrid(false);

    for (int i=0; ui->tablaReporte->rowCount() > 0; i++)
        ui->tablaReporte->removeRow(0);

    for (int i=0; i< nAlumnos; i++){
        int row = ui->tablaReporte->rowCount();
        ui->tablaReporte->insertRow( row );
        for (int j=0; j< columnas;j++)
            ui->tablaReporte->setItem(row, j, new QTableWidgetItem(alumnos[i][j], 0));
    }

    ui->tablaReporte->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
}

void Bus::GuardarReporte()
{
    int tipo_reporte = ui->reporteOpcion->currentIndex();
    QString documento_cadena;
    QTextStream documento_html(&documento_cadena);
    documento_html.setCodec("UTF-8");
    QTableWidget * tabla = ui->tablaReporte;

    const int cantidad_filas = tabla->model()->rowCount();
    const int cantidad_columnas = tabla->model()->columnCount();

    documento_html << "<html>\n";
    documento_html << "<head>\n";
    documento_html << "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\" />\n";
    documento_html << "<style type=\"text/css\">*{font-size: 9px;}table{margin: 5px;}td { padding: 3px; text-align: right;}th { padding: 5px; text-align: center;}</style>";
    documento_html <<  "</head>\n";
    documento_html << "<body>\n<center>";
    switch ( tipo_reporte )
    {
    case 0:
        documento_html << QString("<h1>Reporte de asistencia por ruta «mañana»</h1>");
        break;
    case 1:
        documento_html << QString("<h1>Reporte de asistencia por ruta «tarde»</h1>");
        break;
    case 2:
        documento_html << QString("<h1>Reporte de asistencia por alumno «mañana»</h1>");
        break;
    case 3:
        documento_html << QString("<h1>Reporte de asistencia por alumno «tarde»</h1>");
        break;
    case 4:
        documento_html << QString("<h1>Reporte de inasistencias «mañana»</h1>");
        break;
    case 5:
        documento_html << QString("<h1>Reporte de inasistencias «tarde»</h1>");
        break;
    }
    documento_html << QString("<p>Fecha inicio: %1<br />").arg(ui->reporteInicio->date().toString("dd.MM.yyyy"));
    documento_html << QString("Fecha fin: %1<br />").arg(ui->reporteFin->date().toString("dd.MM.yyyy"));
    documento_html<< "<table style=\"padding:5px;\">\n";

    // headers
    documento_html << "<tr>";
    for (int columna = 0; columna < cantidad_columnas; columna++)
        if (!tabla->isColumnHidden(columna))
            documento_html << QString("<th>%1</th>").arg(tabla->model()->headerData(columna, Qt::Horizontal).toString());
    documento_html << "</tr>\n";

    // data table
    for (int fila = 0; fila < cantidad_filas; fila++) {
        documento_html << "<tr>";
        for (int columna = 0; columna < cantidad_columnas; columna++) {
            if (!tabla->isColumnHidden(columna)) {
                QString data = tabla->model()->data(tabla->model()->index(fila, columna)).toString().simplified();
                documento_html << QString("<td>%1</td>").arg((!data.isEmpty()) ? data : QString("&nbsp;"));
            }
        }
        documento_html << "</tr>\n";
    }
    QTextDocument *documento = new QTextDocument();
    documento->setHtml(documento_cadena);

    QPrinter printer;
    QString s = QFileDialog::getSaveFileName(
                this,
                "Elija el archivo donde se guardará el documento",
                "",
                "Archivo PDF (*.pdf)"
                );
    if (s.isEmpty())
        return;
    printer.setOutputFileName( s );
    printer.setPageSize(QPrinter::Letter);
    printer.setPaperSize(QPrinter::Letter);
    printer.setFullPage( false );
    printer.setPageMargins(20,20,20,20,QPrinter::Millimeter);
    printer.setOutputFormat( QPrinter::PdfFormat );
    documento->print(&printer);

    delete documento;
}

void Bus::ImprimirReporte()
{
    int tipo = ui->reporteOpcion->currentIndex();
    QString strStream;
    QTextStream documento_html(&strStream);
    documento_html.setCodec("UTF-8");
    QTableWidget * tabla = ui->tablaReporte;

    const int rowCount = tabla->model()->rowCount();
    const int columnCount = tabla->model()->columnCount();

    documento_html << "<html>\n";
    documento_html   << "<head>\n";
    documento_html   << "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\" />\n";
    documento_html   << "<style type=\"text/css\">*{font-size: 9px;}table{margin: 5px;}td { padding: 3px; text-align: right;}th { padding: 5px; text-align: center;}</style>";
    documento_html<<  "</head>\n";
    documento_html<< "<body>\n";
    switch ( tipo )
    {
    case 0:
        documento_html << QString("<h1>Reporte de asistencia por ruta «mañana»</h1>");
        break;
    case 1:
        documento_html << QString("<h1>Reporte de asistencia por ruta «tarde»</h1>");
        break;
    case 2:
        documento_html << QString("<h1>Reporte de asistencia por alumno «mañana»</h1>");
        break;
    case 3:
        documento_html << QString("<h1>Reporte de asistencia por alumno «tarde»</h1>");
        break;
    case 4:
        documento_html << QString("<h1>Reporte de inasistencias «mañana»</h1>");
        break;
    case 5:
        documento_html << QString("<h1>Reporte de inasistencias «tarde»</h1>");
        break;
    }
    documento_html << QString("<p>Fecha inicio: %1<br />").arg(ui->reporteInicio->date().toString("dd.MM.yyyy"));
    documento_html << QString("<p>Fecha fin: %1<br />").arg(ui->reporteFin->date().toString("dd.MM.yyyy"));
    documento_html<< "<table style=\"padding:5px;\">\n";

    // headers
    documento_html << "<tr>";
    for (int column = 0; column < columnCount; column++)
        if (!tabla->isColumnHidden(column))
            documento_html << QString("<th>%1</th>").arg(tabla->model()->headerData(column, Qt::Horizontal).toString());
    documento_html << "</tr>\n";

    // data table
    for (int row = 0; row < rowCount; row++) {
        documento_html << "<tr>";
        for (int column = 0; column < columnCount; column++) {
            if (!tabla->isColumnHidden(column)) {
                QString data = tabla->model()->data(tabla->model()->index(row, column)).toString().simplified();
                documento_html << QString("<td>%1</td>").arg((!data.isEmpty()) ? data : QString("&nbsp;"));
            }
        }
        documento_html << "</tr>\n";
    }
    documento_html << "</table>\n" << "</body>\n" << "</html>\n";

    QTextDocument *document = new QTextDocument();
    document->setHtml(strStream);

    QPrinter printer;
    printer.setPageSize(QPrinter::Letter);
    printer.setPaperSize(QPrinter::Letter);
    printer.setPageMargins(20,20,20,20,QPrinter::Millimeter);
    QPrintDialog *dialog = new QPrintDialog(&printer, NULL);
    if (dialog->exec() == QDialog::Accepted) {
        document->print(&printer);
    }

    delete document;
}


/*******************************************************************************
 * Funciones de importación y sincronización de datos
 ******************************************************************************/
void Bus::sincronizarDBs()
{
    dbSync sync;
    sync.sincronizarDB();
}


void Bus::importar_Sige()
{
    QString archivo = QFileDialog::getOpenFileName(
                this,
                "Abrir archivo",
                QDir::currentPath(),
                "Archivo TXT de SIGE (*.txt)" );
    if( archivo.isNull() )
        return;
    else
    {
        csv_lector csv;
        csv.leer_archivo_db( archivo, db_local );
    }
}


/*******************************************************************************
 * Permanencia de opciones
 ******************************************************************************/

// Guarda las opciones del programa de forma permanente
void Bus::guardarOpciones()
{
	QSettings opciones(QSettings::IniFormat, QSettings::UserScope, "Ashtaroth", "AsistenciaBus");
	QStringList rutas;
	int fin = ui->rutasRuta->count();
	for(int index = 0; index < fin; index++)
		rutas.append( ui->rutasRuta->itemText(index) );
	opciones.setValue("rutas", rutas);
	QString archivo = opciones.fileName();
	opciones.beginGroup("DB");
	opciones.setValue("Direccion", ui->syncDireccion->text());
	opciones.setValue("Puerto", ui->syncPuerto->text());
	opciones.setValue("DB", ui->syncDb->text());
	opciones.setValue("Usuario", ui->syncUsuario->text());
	opciones.setValue("Password", ui->syncPassword->text());
	opciones.endGroup();
}

// Carga las opciones del programa
void Bus::cargarOpciones(){
	QSettings opciones(QSettings::IniFormat, QSettings::UserScope, "Ashtaroth", "AsistenciaBus");
	ui->rutasRuta->addItems( opciones.value("rutas").toStringList() );
	ui->autorizarRuta->addItems( opciones.value("rutas").toStringList() );
	opciones.beginGroup("DB");
	ui->syncDireccion->setText( opciones.value("Direccion").toString() );
	ui->syncPuerto->setText( opciones.value("Puerto").toString() );
	ui->syncDb->setText( opciones.value("DB").toString() );
	ui->syncUsuario->setText( opciones.value("Usuario").toString() );
	ui->syncPassword->setText( opciones.value("Password").toString() );
	opciones.endGroup();
}



/*******************************************************************************
 * Funciones de asistencia
 ******************************************************************************/


// Indica si es un viaje de mañana o tarde usando como referencia la hora del sistema
bool Bus::esViajeManana( )
{
    QTime hora_mediodia;
    hora_mediodia.setHMS(12, 00, 00, 00);
    if ( QTime::currentTime() < hora_mediodia )
        return true;
    return false;
}
