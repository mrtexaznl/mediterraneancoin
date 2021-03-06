// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sendcoinsentry.h"
#include "ui_sendcoinsentry.h"

#include "guiutil.h"
#include "bitcoinunits.h"
#include "addressbookpage.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "addresstablemodel.h"

#include <QApplication>
#include <QClipboard>

//
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>


QNetworkAccessManager* nam;
//

SendCoinsEntry::SendCoinsEntry(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SendCoinsEntry),
    model(0)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    ui->payToLayout->setSpacing(4);
#endif
#if QT_VERSION >= 0x040700
    /* Do not move this to the XML file, Qt before 4.7 will choke on it */
    ui->addAsLabel->setPlaceholderText(tr("Enter a label for this address to add it to your address book"));
    ui->payTo->setPlaceholderText(tr("Enter a Mediterraneancoin address (e.g. Mer4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2)"));
#endif
    setFocusPolicy(Qt::TabFocus);
    setFocusProxy(ui->payTo);

    GUIUtil::setupAddressWidget(ui->payTo, this);
}

SendCoinsEntry::~SendCoinsEntry()
{
    delete ui;
}

void SendCoinsEntry::on_pasteButton_clicked()
{
    // Paste text from clipboard into recipient field
    ui->payTo->setText(QApplication::clipboard()->text());
}

void SendCoinsEntry::on_lookupExchangeValueButton_clicked() {
    if(!model)
        return;

    //QString lookupUsername = ui->payAmount->text();
    QString emptyString = "";

    bool validValue = true;

    qint64 value = ui->payAmount->value(&validValue);


    if (!validValue)
    	return;

    QString selectedExchangePlatform = ui->exchangeComboBox->currentText();

    QString  cryptsyMed2BtcUrl("http://lookup.mediterraneancoin.org:9000/cryptsy/med2btc");
    QString  btc38Med2BtcUrl("http://lookup.mediterraneancoin.org:9000/btc38/med2btc");

    QString url("");

    if (selectedExchangePlatform == "Cryptsy")
    	url = cryptsyMed2BtcUrl;
    else if (selectedExchangePlatform == "Bct38")
    	url = btc38Med2BtcUrl;

    if (url == "")
    	return;

    // create custom temporary event loop on stack
    QEventLoop eventLoop;

    // "quit()" the eventl-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));


    QNetworkRequest req( QUrl( url + "" ) );

    QNetworkReply *reply = mgr.get(req);

    eventLoop.exec(); // blocks stack until "finished()" has been called

    if (reply->error() == QNetworkReply::NoError) {
      // Everything is ok => reply->readAll()

        QByteArray bytes = reply->readAll();  // bytes
        QString stringResult(bytes); // string

        double exchangeValue = stringResult.toDouble();

        double medValue = value / 100000000.;
        double btcValue = exchangeValue * value / 100000000;


        // nnn MED = xxx BTC (MED/BTC yyy)

        ui->exchangeResultLabel->setText("" + QString::number(medValue,'f',2) + "MED~=" + QString::number(btcValue,'f',4) +
        		"BTC (rate " + QString::number(exchangeValue,'f',8) + ")" );

        //ui->exchangeResultLabel->setText(stringResult + " " + QString::number(value) + " " + QString::number(btcValue));

      delete reply;
      return;
    }
    else {
      // error... reply->errorString()
      delete reply;
      return;
    }


}

void SendCoinsEntry::on_lookupUserButton_clicked()
{
    if(!model)
        return;
    //

    //ui->payTo->setText("MkVZzg7WdCVu1spaSAr68QuusfaJEacYor");
    QString lookupUsername = ui->addAsLabel->text();
    QString emptyString = "";

    if (lookupUsername == emptyString)
    	return;

    // http://developer.nokia.com/community/wiki/Creating_an_HTTP_network_request_in_Qt
    // http://www.insanefactory.com/2012/08/qt-http-request-in-a-single-function/


    // create custom temporary event loop on stack
    QEventLoop eventLoop;

    // "quit()" the eventl-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    /*
    // the HTTP request
    QNetworkRequest req( QUrl( QString("http://lookup.mediterraneancoin.org:9000/medaddresslookup") ) );

    // some parameters for the HTTP request
    QUrl params;
    params.addQueryItem("screenname", lookupUsername);
    params.addQueryItem("platform", "twitter");
    params.addQueryItem("coin", "MED");

    QNetworkReply *reply = mgr.post(req, params.encodedQuery());
    */

    QString selectedPlatform = ui->platformComboBox->currentText();

    QUrl params;
    params.addQueryItem("screenname", lookupUsername);
    params.addQueryItem("platform", selectedPlatform);
    params.addQueryItem("coin", "MED");
    params.addQueryItem("simple", "true");
    params.addQueryItem("unencode", "true");

    QString econdedParams(params.encodedQuery());

    QNetworkRequest req( QUrl( QString("http://lookup.mediterraneancoin.org:9000/medaddresslookup?") + econdedParams ) );

    QNetworkReply *reply = mgr.get(req);

    eventLoop.exec(); // blocks stack until "finished()" has been called

    if (reply->error() == QNetworkReply::NoError) {
      // Everything is ok => reply->readAll()

        QByteArray bytes = reply->readAll();  // bytes
        QString string(bytes); // string

        ui->payTo->setText(string);

      delete reply;
      return;
    }
    else {
      // error... reply->errorString()
      delete reply;
      return;
    }

    // if lookupUsername != ""
    // perform a get request....
    // take the label content... perform a lookup query

    /*
    {
        ui->payTo->setText(dlg.getReturnValue());
        ui->payAmount->setFocus();
    }
    */


}



void SendCoinsEntry::on_addressBookButton_clicked()
{
    if(!model)
        return;
    AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::SendingTab, this);
    dlg.setModel(model->getAddressTableModel());
    if(dlg.exec())
    {
        ui->payTo->setText(dlg.getReturnValue());
        ui->payAmount->setFocus();
    }
}

void SendCoinsEntry::on_payTo_textChanged(const QString &address)
{
    if(!model)
        return;
    // Fill in label from address book, if address has an associated label
    QString associatedLabel = model->getAddressTableModel()->labelForAddress(address);
    if(!associatedLabel.isEmpty())
        ui->addAsLabel->setText(associatedLabel);
}

void SendCoinsEntry::setModel(WalletModel *model)
{
    this->model = model;

    if(model && model->getOptionsModel())
        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));

    connect(ui->payAmount, SIGNAL(textChanged()), this, SIGNAL(payAmountChanged()));


    clear();
}

void SendCoinsEntry::setRemoveEnabled(bool enabled)
{
    ui->deleteButton->setEnabled(enabled);
}

void SendCoinsEntry::clear()
{
    ui->payTo->clear();
    ui->addAsLabel->clear();
    ui->payAmount->clear();
    ui->payTo->setFocus();
    // update the display unit, to not use the default ("BTC")
    updateDisplayUnit();
}

void SendCoinsEntry::on_deleteButton_clicked()
{
    emit removeEntry(this);
}

bool SendCoinsEntry::validate()
{
    // Check input validity
    bool retval = true;

    if(!ui->payAmount->validate())
    {
        retval = false;
    }
    else
    {
        if(ui->payAmount->value() <= 0)
        {
            // Cannot send 0 coins or less
            ui->payAmount->setValid(false);
            retval = false;
        }
    }

    if(!ui->payTo->hasAcceptableInput() ||
       (model && !model->validateAddress(ui->payTo->text())))
    {
        ui->payTo->setValid(false);
        retval = false;
    }

    return retval;
}

SendCoinsRecipient SendCoinsEntry::getValue()
{
    SendCoinsRecipient rv;

    rv.address = ui->payTo->text();
    rv.label = ui->addAsLabel->text();
    rv.amount = ui->payAmount->value();

    return rv;
}

QWidget *SendCoinsEntry::setupTabChain(QWidget *prev)
{
    QWidget::setTabOrder(prev, ui->payTo);
    QWidget::setTabOrder(ui->payTo, ui->addressBookButton);
    QWidget::setTabOrder(ui->addressBookButton, ui->pasteButton);
    QWidget::setTabOrder(ui->pasteButton, ui->deleteButton);
    QWidget::setTabOrder(ui->deleteButton, ui->addAsLabel);
    return ui->payAmount->setupTabChain(ui->addAsLabel);
}

void SendCoinsEntry::setValue(const SendCoinsRecipient &value)
{
    ui->payTo->setText(value.address);
    ui->addAsLabel->setText(value.label);
    ui->payAmount->setValue(value.amount);
}

void SendCoinsEntry::setAddress(const QString &address)
{
    ui->payTo->setText(address);
    ui->payAmount->setFocus();
}

bool SendCoinsEntry::isClear()
{
    return ui->payTo->text().isEmpty();
}

void SendCoinsEntry::setFocus()
{
    ui->payTo->setFocus();
}

void SendCoinsEntry::updateDisplayUnit()
{
    if(model && model->getOptionsModel())
    {
        // Update payAmount with the current unit
        ui->payAmount->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
    }
}
