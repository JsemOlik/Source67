#pragma once

#include "Base.h"
#include "Renderer/Entity.h"
#include <deque>
#include <memory>

namespace S67 {

    class Command {
    public:
        virtual ~Command() = default;
        virtual void Undo() = 0;
        virtual void Redo() = 0;
    };

    class TransformCommand : public Command {
    public:
        TransformCommand(Ref<Entity> entity, const Transform& oldTransform, const Transform& newTransform)
            : m_Entity(entity), m_OldTransform(oldTransform), m_NewTransform(newTransform) {}

        void Undo() override {
            m_Entity->Transform = m_OldTransform;
        }

        void Redo() override {
            m_Entity->Transform = m_NewTransform;
        }

    private:
        Ref<Entity> m_Entity;
        Transform m_OldTransform;
        Transform m_NewTransform;
    };

    class TextureCommand : public Command {
    public:
        TextureCommand(Ref<Entity> entity, Ref<Texture2D> oldTexture, Ref<Texture2D> newTexture)
            : m_Entity(entity), m_OldTexture(oldTexture), m_NewTexture(newTexture) {}

        void Undo() override {
            m_Entity->MaterialTexture = m_OldTexture;
        }

        void Redo() override {
            m_Entity->MaterialTexture = m_NewTexture;
        }

    private:
        Ref<Entity> m_Entity;
        Ref<Texture2D> m_OldTexture;
        Ref<Texture2D> m_NewTexture;
    };

    class UndoSystem {
    public:
        void Push(Scope<Command> command) {
            command->Redo(); // Assumes command is already in "new" state, or just pushes it
            m_UndoStack.push_back(std::move(command));
            m_RedoStack.clear();

            if (m_UndoStack.size() > 30) {
                m_UndoStack.pop_front();
            }
        }

        // External push (already executed)
        void AddCommand(Scope<Command> command) {
            m_UndoStack.push_back(std::move(command));
            m_RedoStack.clear();

            if (m_UndoStack.size() > 30) {
                m_UndoStack.pop_front();
            }
        }

        void Undo() {
            if (m_UndoStack.empty()) return;

            auto command = std::move(m_UndoStack.back());
            m_UndoStack.pop_back();
            command->Undo();
            m_RedoStack.push_back(std::move(command));
        }

        void Redo() {
            if (m_RedoStack.empty()) return;

            auto command = std::move(m_RedoStack.back());
            m_RedoStack.pop_back();
            command->Redo();
            m_UndoStack.push_back(std::move(command));
        }

        void Clear() {
            m_UndoStack.clear();
            m_RedoStack.clear();
        }

    private:
        std::deque<Scope<Command>> m_UndoStack;
        std::deque<Scope<Command>> m_RedoStack;
    };

}
