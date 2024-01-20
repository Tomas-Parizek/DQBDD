/*
 * This file is part of DQBDD.
 *
 * Copyright 2020, 2021 Juraj Síč
 *
 * DQBDD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * DQBDD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public License
 * along with DQBDD. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dqbddmodel.hpp"
#include <iostream>

namespace dqbdd {

/*Model::Model(Cudd& mgr, BDD matrix) : mgr(mgr)
{
    using namespace std;
    path = vector<bool>();
    DdNode * root = matrix.getNode();
    searchModel(root);

    for (bool i : path)
        cout << i << " ";
    cout << endl;
}
*/

Model::Model(Cudd &mgr) : mgr(mgr)
{
    evaluation = new Evaluation();
}


void Model::evaluateFormula(BDD formula, VariableSet variableSet)
{
    using namespace std;
    DdNode * root = formula.getNode();

    if (formula.IsOne())
    {}
    else
    {
        searchModel(formula.getNode(), getEvaluation());
    }

    for (auto &variable : variableSet)
    {
        if (evaluation->count(variable.getId()) == 0)
            evaluation->emplace(variable.getId(), mgr.bddZero());
    }
}

void Model::searchModel(DdNode * node, Evaluation * evaluation)
{
    using namespace std;

    if (Cudd_IsConstant(Cudd_E(node)) && evaluateTerminalNode(Cudd_E(node)))
    {
        evaluation->emplace(Cudd_NodeReadIndex(node), mgr.bddZero());
        substituteInAll(Cudd_NodeReadIndex(node), mgr.bddZero());
    }
    else if (Cudd_IsConstant(Cudd_T(node)) && evaluateTerminalNode(Cudd_T(node)))
    {
        evaluation->emplace(Cudd_NodeReadIndex(node), mgr.bddOne());
        substituteInAll(Cudd_NodeReadIndex(node), mgr.bddOne());
    }
    else if (!Cudd_IsConstant(Cudd_E(node))) // Not equal to zero on left
    {
        searchModel(Cudd_E(node), evaluation);
        evaluation->emplace(Cudd_NodeReadIndex(node), mgr.bddZero());
        substituteInAll(Cudd_NodeReadIndex(node), mgr.bddZero());
    }
    else
    {
        searchModel(Cudd_T(node), evaluation);
        evaluation->emplace(Cudd_NodeReadIndex(node), mgr.bddOne());
        substituteInAll(Cudd_NodeReadIndex(node), mgr.bddOne());
    }
}

BDD Model::getVarEvaluation(unsigned var)
{
    // Probably redundant if statement
    if (evaluation->count(var) != 0)
        return (*evaluation)[var];
    
    else
    {
        evaluation->emplace(var, mgr.bddZero());
        return (*evaluation)[var];
    }
}

bool Model::evaluateTerminalNode(DdNode * node)
{
    return BDD(mgr, node).IsOne();
}

Evaluation * Model::getEvaluation()
{
    return evaluation;
}

void Model::printModel(Evaluation &evaluation)
{
    using namespace std;
    if (evaluation.size() == 0)
    {
        cout << "{}" << endl;
        return;
    }

    cout << "{";
    for (auto &i : evaluation)
    {
        cout << "x" << i.first << " ↦ " << i.second << ", ";
    }
    cout << "\b\b}" << endl;
}

void Model::printModel()
{
    printModel(*this->evaluation);
}

bool Model::isVarInModel(unsigned variable)
{
    return evaluation->count(variable) != 0;
}

bool Model::isVarInModel(Variable variable)
{
    return evaluation->count(variable.getId()) != 0;
}

void Model::createSkolemFunction(UniversalReduction universalReduction)
{
    using namespace std;

    unsigned originalVarId = get<0>(universalReduction).getId();
    unsigned newVarId = get<1>(universalReduction).getId();

    BDD originalVarValue = getVarEvaluation(originalVarId);
    BDD newVarValue = getVarEvaluation(newVarId);
    BDD uvarValue = get<2>(universalReduction);

    /*BDD SkolemFunction = (!uvarValue & originalVarValue) |
                         ( uvarValue & newVarValue);*/
    
    BDD SkolemFunction = uvarValue.Ite(newVarValue, originalVarValue);
    
    evaluation->erase(originalVarId);
    evaluation->erase(newVarId);
    evaluation->emplace(originalVarId, SkolemFunction);
}

BDD Model::substitute(Cudd & mgr, BDD formula, BDD substitution, int variable)
{
    return BDD(mgr, Cudd_bddCompose(mgr.getManager(), formula.getNode(), substitution.getNode(), variable));
}

BDD Model::substitute(Cudd & mgr, BDD formula, BDD substitution, Variable variable)
{
    return substitute(mgr, formula, substitution, variable.getId());
}

BDD Model::substitute(Cudd & mgr, BDD formula, BDD substitution, BDD variable)
{
    return substitute(mgr, formula, substitution, Cudd_NodeReadIndex(variable.getNode()));
}

void Model::substituteInAll(unsigned variable, BDD skolemFunction)
{
    for (auto &i : *evaluation)
    {
        i.second = substitute(mgr, i.second, skolemFunction, variable);
    }
}

};