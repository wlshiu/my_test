
#include <stdint.h>
#include <stdio.h>
#include <locale.h>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDebug>

static QString      g_InputFilePath;

int main(int argc, char* argv[])
{
    int                 rval = 0;
    QCoreApplication    appcore(argc, argv);

    g_InputFilePath.clear();

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
            g_InputFilePath.append(value);
            i++;
        }
    }

    do {
        QFile       fIn(g_InputFilePath);
        QByteArray  inData;

        if( !fIn.open(QIODevice::ReadOnly) )
        {
            qDebug("Can't open input file !");
            break;
        }

        inData = fIn.readAll();
        fIn.close();

        qDebug("filesize= %d\n", inData.size());

        uint8_t     *pCur = (uint8_t*)inData.data();
        int         len = inData.size();
        uintptr_t   addr = (uintptr_t)pCur;

        for(int i = 0; i < len; i++)
        {
            if( (i & 0xF) == 8 )
                printf(" -");
            else if( !(i & 0xF) )
            {
                printf("\n%08X |", addr);
                addr += 16;
            }

            printf(" %02X", pCur[i]);
        }
        printf("\n\n");
    } while(0);


    return rval;
}
