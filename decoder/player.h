#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>

class PlayerImpl : public QObject
{
    Q_OBJECT
public:
    PlayerImpl(QObject* parent = nullptr) {}
    virtual ~PlayerImpl() {}
};

class AudioPlayer : public PlayerImpl
{

};

#endif // PLAYER_H
