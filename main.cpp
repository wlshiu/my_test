#include <stdio.h>
#include <locale.h>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDebug>

int main(int argc, char* argv[])
{
    int                 rval = 0;
    QCoreApplication    appcore(argc, argv);

    for(int i = 1; i < argc; i++)
    {
        QString    option = appcore.arguments().at(i);

        if( option.compare("--key", Qt::CaseInsensitive) == 0 )
        {
            QString    value = appcore.arguments().at(i + 1);

            qDebug() << "key: " << value << endl;
            i++;
        }
        else if( option.compare("--input", Qt::CaseInsensitive) == 0 )
        {
            QString    value = appcore.arguments().at(i + 1);
            qDebug() << "input: " << value << endl;
            i++;
        }
    }

    return rval;
}
