#include <QtGui/QApplication>
#include "bus.h"

int main(int argc, char *argv[])
{
		QApplication a(argc, argv);
		Bus w;
		w.show();

		return a.exec();
}
