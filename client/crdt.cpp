#include <iostream>
#include <QtDebug>
#include "crdt.h"
#include "symbol.h"


int Crdt::maxnum=100;

int Crdt::counter=0;
extern socketManager *sock;

Crdt::Crdt(){}

std::vector<int> createFractional(std::vector<int> preceding, std::vector<int> following, std::vector<int> &tmp, const int maxnum){
    int prec, foll;

    prec=preceding.at(0);
    foll=following.at(0);

    int diff=std::abs(foll-prec);

    if(diff==1 || diff==0){
        if(preceding.size()==1){
            preceding.at(0)=0;
        }
        else{
            preceding.erase(preceding.begin());
        }
        if(following.size()==1){
            following.at(0)=maxnum;
        }
        else{
            following.erase(following.begin());
        }

        createFractional(preceding, following, tmp, maxnum);
    }
    else if(diff > 1){
        tmp.push_back(prec+(foll-prec)/2);
        return tmp;
    }

    tmp.insert(tmp.begin(), prec);          //inserimento a ritroso
    return tmp;
}


std::vector<Symbol>::iterator Crdt::localInsert(QChar value, int precedingC, int followingC){
    //dimensione del vettore di simboli
    size_t symbolsSize = this->symbols.size();
    std::vector<int> following;
    std::vector<int> preceding;
    std::vector<int> fractionalPos;

    //prendo il simbolo nuovo
    if(precedingC==-1){
        qDebug() << "sono all'inserimento in testa";

        if(symbolsSize==0){ //primissimo inserimento
            fractionalPos = std::vector<int>{maxnum/2};
        }
        else{
            following = this->symbols[followingC].getPosizione();
            createFractional({0}, following, fractionalPos, Crdt::maxnum);
        }
    }
    else{
        if(followingC == (int)symbolsSize){         // inserimento in coda
                preceding = this->symbols[precedingC].getPosizione();
                following=std::vector<int>{maxnum};
                createFractional(preceding, following, fractionalPos, Crdt::maxnum);
//                qDebug() << "fractionalpos: " << fractionalPos;;
        }
        else{ // cioè sono in mezzo
            //mi salvo le posizioni frazionarie di quello prima e di quello dopo
            preceding = this->symbols[precedingC].getPosizione();
            following = this->symbols[followingC].getPosizione();
            createFractional(preceding, following, fractionalPos, Crdt::maxnum);
//            qDebug() << "fractionalpos: " << fractionalPos;
        }
    }

    Symbol symbolToInsert(value, fractionalPos, this->getSiteId(), this->getCounterAndIncrement());
//
    auto pos = this->symbols.insert(this->symbols.begin()+followingC, symbolToInsert);
    return pos;
}

Message Crdt::localErase(int position){ //la riscrivo il 22/10 per ricerca idempotenza
    std::vector<Symbol>::iterator i = this->symbols.begin()+position;
    //symbolo da eliminare
    Symbol symbol(i->getValue(), i->getPosizione(), i->getSiteId(), i->getCounter());

    /* creo messaggio */
    Message m;
    m.setAction('D');
    m.setSymbol(symbol);
    //incremento counter delle operazioni del crdt
    this->incrementCounter();
    //elimino localmente
    this->symbols.erase(i);
    //invio messaggio al server
    return m;
}


int Crdt::getSiteId(){
    return this->siteId;
}

void Crdt::setSiteId(int s)
{
    siteId=s;
}

int Crdt::getCounter(){
    return Crdt::counter;
}

std::vector<Symbol> Crdt::getSymbols(){
    return symbols;
}

void Crdt::setSymbols(std::vector<Symbol> vect)
{
    symbols=vect;
}

int Crdt::getCounterAndIncrement(){
    return ++Crdt::counter;
}

void Crdt::incrementCounter(){
    Crdt::counter++;
}


int Crdt::remoteinsert(Symbol s){
    int min=0,max=symbols.size()-1,middle=(max+min)/2,pos;
    std::vector<int> index=s.getPosizione();
    std::vector<int> tmp;
    std::vector<Symbol>::iterator it;
    //controllo se è vuoto
    if(symbols.size()==0){
        symbols.push_back(s);
        return 0;
    }
    //controllo se è ultimo
    else if(this->compare(s,symbols[max])>0){
        symbols.push_back(s);
        return max+1;
    }
    //controllo se è primo
    else if(this->compare(s,symbols[0])<0){
        it=symbols.begin();
        symbols.insert(it,s);
        return min;
    }
    //è in mezzo
    while(max-min>1){
       pos=this->compare(s,symbols[middle]);
       if(pos>0){
           min=middle;
       }
       else if(pos<0){
           max=middle;
       }
       else if(pos == 0) {
           qDebug()<<"errore: inserimento stesso carattere";
           return -1;
       }
       middle=(max+min)/2;
    }
    it=symbols.begin();
    pos=this->compare(s,symbols[min]);
    if(pos>0){
        //inserisco dopo il min
        symbols.insert(it+min+1,s);
        return min+1;
    }
    if(pos<0){
        //inserisco prima del min
        symbols.insert(it+min-1,s);
        return min-1;
    }

}

int Crdt::remotedelete(Symbol s){
    int min=0,max=symbols.size()-1,middle=(max+min)/2,pos;
    std::vector<int> index=s.getPosizione();
    std::vector<int> tmp;
    std::vector<Symbol>::iterator it;
    it=symbols.begin();
    //controllo se è ultimo
    if(this->compare(s,symbols[max])==0){
            symbols.erase(it+max);
            return max;
}
    //controllo se è primo
    if(this->compare(s,symbols[min])==0){
            symbols.erase(it+min);
            return min;
        }
    while(max-min>1){
       pos=this->compare(s,symbols[middle]);
       if(pos>0){
           min=middle;
       }
       else if(pos<0){
           max=middle;
       }
       if(pos==0){
           symbols.erase(it+middle);
           break;
       }
       middle=(max+min)/2;
    }

    if(pos!=0) {
        if(pos>0){

            symbols.erase(it+max);
            return max;
        }
        else{
            symbols.erase(it+min);
            return min;
        }
    }
    return middle;
}

void Crdt::setAlline(int pos, QChar a)
{
    symbols[pos].setAlign(a);
}

int Crdt::compare(Symbol s1, Symbol s2){
    int len1=s1.getPosizione().size();
    int len2=s2.getPosizione().size();
    int res=0;
    for(int i=0;i<std::min(len1,len2);i++){

        if(s1.getPosizione()[i]>s2.getPosizione()[i]){
            res=1;
            break;
        }
        if(s1.getPosizione()[i]<s2.getPosizione()[i]){
            res=-1;
            break;
        }
    }
    if(res==0){
        if(len1>len2){
            res=1;
        }else
        if(len1<len2){
            res=-1;
        }
        else if(s1.getSiteId()<s2.getSiteId())
            res=1;
        else if(s1.getSiteId()>s2.getSiteId())
            res = -1;
        else if(s1.getSiteId()==s2.getSiteId()){
            res = 0;
        }
    }
    return res;
}

Crdt::~Crdt(){

}

