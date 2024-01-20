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

#include <vector>
#include <unordered_map>
#include <cuddObj.hh>
#include "dqbddvariable.hpp"
#include "quantifiedvariablesmanipulator.hpp"

namespace dqbdd {

using Evaluation = std::unordered_map<unsigned, BDD>;
using UniversalReduction = std::tuple<Variable, Variable, Variable>;
enum QuantifierType {universal, existential};

class Model {
private:
    Evaluation * evaluation;
    Cudd & mgr;
    void searchModel(DdNode *, Evaluation *);
    bool evaluateTerminalNode(DdNode *);

public:
    Model(Cudd &);
    ~Model() = default;
    void evaluateFormula(BDD formula, VariableSet variableSet);

    BDD getVarEvaluation(unsigned);
    Evaluation * getEvaluation();
    static void printModel(Evaluation&);
    void printModel();
    bool isVarInModel(unsigned);
    bool isVarInModel(Variable);
    
    void createSkolemFunction(UniversalReduction);
    static BDD substitute(Cudd & mgr, BDD formula, BDD substitution, int variable);
    static BDD substitute(Cudd & mgr, BDD formula, BDD substitution, Variable variable);
    static BDD substitute(Cudd & mgr, BDD formula, BDD substitution, BDD variable);

    void substituteInAll(unsigned variable, BDD skolemFunction);
};

}; // dqbdd