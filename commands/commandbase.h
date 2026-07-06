#ifndef COMMANDBASE_H
#define COMMANDBASE_H

#include <QUndoCommand>

class CircuitScene;

class CommandBase : public QUndoCommand
{
public:
    explicit CommandBase(
        CircuitScene *scene,
        QUndoCommand *parent = nullptr);

protected:
    CircuitScene *m_scene;
};

#endif // COMMANDBASE_H