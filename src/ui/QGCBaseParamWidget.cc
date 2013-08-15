#include "QGCBaseParamWidget.h"

#include <QFileDialog>
#include <QFile>
#include <QVariant>
#include <QTextStream>>

#include "QGCUASParamManager.h"
#include "UASInterface.h"


QGCBaseParamWidget::QGCBaseParamWidget(QWidget *parent) :
    QWidget(parent),
    mav(NULL),
    paramMgr(NULL),
    updatingParamNameLock("")
{
}

QGCBaseParamWidget* QGCBaseParamWidget::initWithUAS(UASInterface *uas)
{
    setUAS(uas);
    layoutWidget();

    return this;
}

void QGCBaseParamWidget::setUAS(UASInterface* uas)
{
    if (mav) {
        //TODO disconnect any connections as needed
        disconnectViewSignalsAndSlots();
        disconnectFromParamManager();
        clearOnboardParamDisplay();
        clearPendingParamDisplay();
    }
    mav = uas;
    connectToParamManager();
    connectViewSignalsAndSlots();

    paramMgr->requestParameterListIfEmpty();

}


void QGCBaseParamWidget::connectToParamManager()
{
    //TODO route via paramManager instead?
    // Listen to updated param signals from the data model
    connect(paramMgr->dataModel(), SIGNAL(parameterUpdated(int, QString , QVariant )),
            this, SLOT(handleOnboardParamUpdate(int,QString,QVariant)));

    connect(paramMgr->dataModel(), SIGNAL(pendingParamUpdate(int , const QString&, QVariant , bool )),
            this, SLOT(handlePendingParamUpdate(int , const QString& ,  QVariant, bool )));

    // Listen for param list reload finished
    connect(paramMgr, SIGNAL(parameterListUpToDate()),
            this, SLOT(handleOnboardParameterListUpToDate()));

    // Listen to communications status messages so we can display them
    connect(paramMgr, SIGNAL(parameterStatusMsgUpdated(QString,int)),
            this, SLOT(handleParamStatusMsgUpdate(QString , int )));
}


void QGCBaseParamWidget::disconnectFromParamManager()
{
    disconnect(paramMgr->dataModel(), SIGNAL(parameterUpdated(int, QString , QVariant )),
            this, SLOT(handleOnboardParamUpdate(int,QString,QVariant)));

    disconnect(paramMgr->dataModel(), SIGNAL(pendingParamUpdate(int , const QString&, QVariant , bool )),
            this, SLOT(handlePendingParamUpdate(int , const QString& ,  QVariant, bool )));

    disconnect(paramMgr, SIGNAL(parameterListUpToDate()),
            this, SLOT(handleOnboardParameterListUpToDate()));

    // Listen to communications status messages so we can display them
    disconnect(paramMgr, SIGNAL(parameterStatusMsgUpdated(QString,int)),
            this, SLOT(handleParamStatusMsgUpdate(QString , int )));
}



void QGCBaseParamWidget::requestOnboardParamsUpdate()
{
    paramMgr->requestParameterList();
}


void QGCBaseParamWidget::saveParametersToFile()
{
    if (!mav)
        return;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "./parameters.txt", tr("Parameter File (*.txt)"));
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream outstream(&file);
    paramMgr->writeOnboardParamsToStream(outstream,mav->getUASName());
    file.close();
}


void QGCBaseParamWidget::loadParametersFromFile()
{
    if (!mav)
        return;

    QString fileName = QFileDialog::getOpenFileName(this, tr("Load File"), ".", tr("Parameter file (*.txt)"));
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    paramMgr->readPendingParamsFromStream(in);
    file.close();
}


