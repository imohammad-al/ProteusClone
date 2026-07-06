#ifndef ADDWIRECOMMAND_H
#define ADDWIRECOMMAND_H

#include <QUndoCommand>

class CircuitScene;
class Wire;

class AddWireCommand : public QUndoCommand
{
public:
    AddWireCommand(CircuitScene *scene,
                   Wire *wire,
                   QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    CircuitScene *m_scene;
    Wire *m_wire;
    bool m_firstRedo = true;
};

#endif