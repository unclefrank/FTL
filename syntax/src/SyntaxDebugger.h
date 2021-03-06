/*
 * Copyright (C) 2007-2015 Frank Mertens.
 *
 * Use of this source is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef FLUXSYNTAX_SYNTAXDEBUGGER_H
#define FLUXSYNTAX_SYNTAXDEBUGGER_H

#include <flux/String>
#include <flux/Map>
#include <flux/syntax/syntax>

namespace flux {
namespace syntax {

class SyntaxDebugNode: public SyntaxNode {
public:
    virtual int matchNext(ByteArray *text, int i, Token *parentToken, SyntaxState *state) const {
        return entry()->matchNext(text, i, parentToken, state);
    }

    virtual SyntaxNode *succ(SyntaxNode *node) const {
        return SyntaxNode::parent() ? SyntaxNode::parent()->succ(SyntaxNode::self()) : null<SyntaxNode>();
    }

    virtual int matchLength() const {
        return entry() ? entry()->matchLength() : -1;
    }

    virtual const char *declType() const = 0;
    virtual void printAttributes(String indent) {}

    virtual void printNext(String indent = "");

    inline SyntaxNode *entry() const { return SyntaxNode::firstChild(); }

protected:
    SyntaxDebugNode(SyntaxDebugger *debugger, SyntaxNode *newNode)
        : debugger_(debugger)
    {
        appendChild(newNode);
    }

    void printBranch(SyntaxNode *node, String indent);

    String superIndent(String indent) const;
    String subIndent(String indent) const;

    SyntaxDebugger *debugger_;
};

class SyntaxDebugger: public SyntaxDebugFactory
{
public:
    inline static Ref<SyntaxDebugger> create(String indent = "\t") {
        return new SyntaxDebugger(indent);
    }

    virtual SyntaxNode *produce(SyntaxNode *newNode, const char *nodeType);
    void printDefinition(bool omitUnusedRules = false);

    typedef DefinitionNode::StateIdByName StateIdByName;
    typedef Map<int, String> StateNameById;

    Ref<StateNameById> newReverseMap(StateIdByName *stateIdByName);
    StateNameById *flagNameById();
    StateNameById *captureNameById();

private:
    friend class DefinitionNode;
    friend class SyntaxDebugNode;

    SyntaxDebugger(String indent);

    static void determineRulesInUse(RuleNode *rule);

    class NodeFactory: public Object {
    public:
        virtual SyntaxNode *produce(SyntaxNode *newNode) = 0;
    };

    template<class DebugNodeType>
    class DebugNodeFactory: public NodeFactory {
    public:
        DebugNodeFactory(SyntaxDebugger *debugger)
            : debugger_(debugger)
        {}
        virtual SyntaxNode *produce(SyntaxNode *newNode) {
            return new DebugNodeType(debugger_, newNode);
        }
    private:
        SyntaxDebugger *debugger_;
    };

    typedef PrefixTree< char, Ref<NodeFactory> > FactoryByNodeType;
    Ref<FactoryByNodeType> factoryByNodeType_;

    String indent_;

    Ref<StateNameById> flagNameById_;
    Ref<StateNameById> captureNameById_;
};

}} // namespace flux::syntax

#endif // FLUXSYNTAX_SYNTAXDEBUGGER_H
