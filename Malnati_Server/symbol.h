#ifndef SYMBOL_H
#define SYMBOL_H

#include <vector>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <iostream>


class Symbol
{

private:
    QChar value;
    std::vector<int> position;
    int siteId;
    int counter;

public:
    Symbol(std::vector<int> position, int siteId, int counter);
    Symbol(QChar value, std::vector<int> position, int siteId, int counter);
    Symbol();

    std::vector<int> getPosition() const;
    void setPosition(const std::vector<int> &value);
    int getSiteId() const;
    void setSiteId(int value);
    int getCounter() const;
    void setCounter(int value);
    QChar getValue() const;
    void setValue(QChar v);
    static Symbol fromJson(const QJsonObject &JsonObj);
    bool operator < (const Symbol &other) const;
};

#endif // SYMBOL_H

