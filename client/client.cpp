#include "client.h"

Client::Client()
{
}
Client::~Client(){
    delete mw;
    if(closed==false){
        delete crdt;
    }
    delete sockm;
}

bool Client::Login()
{
    LoginWindow lw;
    Registration rw;
    AccountInterface ai;



//    sockm = new socketManager(QUrl(QStringLiteral("ws://localhost:1234")));



    sockm = new socketManager(QUrl(QStringLiteral("ws://angelofloridia.ddns.net:8080")));

    mw = new MainWindow();

    connect(&lw,&LoginWindow::closeMw,mw,&MainWindow::closeMw);
    connect(&lw,&LoginWindow::openRw,&rw,&Registration::openRw);
    connect(&lw,&LoginWindow::closeRw,&rw,&Registration::closeRw);
    connect(&rw,&Registration::sendError,&lw,&LoginWindow::receiveErrorReg);
    connect(sockm,&socketManager::receiveRegistration,&lw,&LoginWindow::receiveRegistration);
    connect(&lw, &LoginWindow::sendMessage, sockm, &socketManager::messageToServer);
    connect(&rw, &Registration::sendMessage, sockm, &socketManager::messageToServer);
    connect(sockm, &socketManager::receivedLogin, &lw, &LoginWindow::receivedLogin);
    connect(sockm, &socketManager::loggedin,&lw, &LoginWindow::loggedin);
    connect(sockm, &socketManager::receivedInfoAccount, mw, &MainWindow::receivedInfoAccount);
    connect(sockm, &socketManager::receivedFile, mw, &MainWindow::receivedFile);
    connect(sockm, &socketManager::receivedURIerror, mw, &MainWindow::receiveURIerror);
    connect(sockm, &socketManager::setSiteId,&lw,&LoginWindow::receivedSiteId);
    connect(sockm, &socketManager::showUsers, mw, &MainWindow::showUsers);
    connect(sockm, &socketManager::receiveNewImage, mw, &MainWindow::receiveNewImageMW);
    connect(sockm, &socketManager::receiveNewPsw, mw, &MainWindow::receiveNewPswMW);

    lw.exec();

    if(lw.getIsLogin()==true){
         mw->show();
         crdt=new Crdt();
         connect(mw, &MainWindow::newTextEdit, this, &Client::receive_textEdit);
         connect(mw, &MainWindow::closeTextEdit, this, &Client::closeTextEdit);
         connect(mw, &MainWindow::sendImage,sockm,&socketManager::messageToServer);
         connect(mw, &MainWindow::sendPwd, sockm, &socketManager::messageToServer);
         connect(mw,&MainWindow::sendMessage,sockm,&socketManager::binaryMessageToServer);
         connect(mw,&MainWindow::sendTextMessage,sockm,&socketManager::messageToServer);
         return true;

    }
    else{
       closed=true;
        return false;
    }
}

void Client::receive_textEdit(TextEdit *t,int s){
    this->textList.insert("prova1",t);
    this->crdt->setSiteId(s);
    t->setCrdt(this->crdt);
    t->setSocketM(Client::sockm);
    //da disconnettere!
    connect(sockm, &socketManager::newMessage, t, &TextEdit::receiveSymbol, Qt::UniqueConnection);

}

void Client::closeTextEdit(TextEdit *t){
    disconnect(sockm, &socketManager::newMessage, t, &TextEdit::receiveSymbol);
}



