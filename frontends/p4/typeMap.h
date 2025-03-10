/*
Copyright 2013-present Barefoot Networks, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef FRONTENDS_P4_TYPEMAP_H_
#define FRONTENDS_P4_TYPEMAP_H_

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "frontends/common/programMap.h"
#include "frontends/p4/typeChecking/typeSubstitution.h"

namespace P4 {
/**
Maps nodes to their canonical types.
Not all Node objects have types.
The type of each node is maintained in the typeMap.
Objects that have a type in the map:
- all Expression objects (includes Literals)
- function parameters
- variables and constant declarations
- functors (control, parser, etc.)
- functions
- interfaces
- enum fields (pointing to the enclosing enum)
- error (pointing to the error type)
- type declarations - map name to the actual type
*/
class TypeMap final : public ProgramMap {
    // We want to have the same canonical type for two
    // different tuples, lists, stacks, or p4lists with the same signature.
    std::vector<const IR::Type *> canonicalTuples;
    std::vector<const IR::Type *> canonicalStacks;
    std::vector<const IR::Type *> canonicalP4lists;
    std::vector<const IR::Type *> canonicalLists;

    // Map each node to its canonical type
    absl::flat_hash_map<const IR::Node *, const IR::Type *, Util::Hash> typeMap;
    // All left-values in the program.
    absl::flat_hash_set<const IR::Expression *, Util::Hash> leftValues;
    // All compile-time constants.  A compile-time constant
    // is not necessarily a constant - it could be a directionless
    // parameter as well.
    absl::flat_hash_set<const IR::Expression *, Util::Hash> constants;
    // For each type variable in the program the actual
    // type that is substituted for it.
    TypeVariableSubstitution allTypeVariables;

    // checks some preconditions before setting the type
    void checkPrecondition(const IR::Node *element, const IR::Type *type) const;

 public:
    TypeMap() : ProgramMap("TypeMap"), strictStruct(false) {}

    /// If true we require structs to have the same name to be
    /// equivalent, if false only that the have the same fields.
    bool strictStruct;
    void setStrictStruct(bool value) { strictStruct = value; }
    bool contains(const IR::Node *element) { return typeMap.count(element) != 0; }
    void setType(const IR::Node *element, const IR::Type *type);
    const IR::Type *getType(const IR::Node *element, bool notNull = false) const;
    // unwraps a TypeType into its contents
    const IR::Type *getTypeType(const IR::Node *element, bool notNull) const;
    void dbprint(std::ostream &out) const override;
    void clear();
    bool isLeftValue(const IR::Expression *expression) const {
        return leftValues.count(expression) > 0;
    }
    bool isCompileTimeConstant(const IR::Expression *expression) const;
    size_t size() const { return typeMap.size(); }

    void setLeftValue(const IR::Expression *expression);
    void cloneExpressionProperties(const IR::Expression *to, const IR::Expression *from);
    void setCompileTimeConstant(const IR::Expression *expression);
    void addSubstitutions(const TypeVariableSubstitution *tvs);
    const IR::Type *getSubstitution(const IR::ITypeVar *var) {
        return allTypeVariables.lookup(var);
    }
    const TypeVariableSubstitution *getSubstitutions() const { return &allTypeVariables; }

    /// Check deep structural equivalence; defined between canonical types only.
    /// @param strict  If true use strict equivalence checks, irrespective
    ///        of the strictStruct flag, else use the strictStruct flag value.
    bool equivalent(const IR::Type *left, const IR::Type *right, bool strict = false) const;
    /// This is the same as equivalence, but it also allows some legal
    /// implicit conversions, such as a tuple type to a struct type, which
    /// is used when initializing a struct with a list expression.
    bool implicitlyConvertibleTo(const IR::Type *from, const IR::Type *to) const;

    // Used for tuples and stacks only
    const IR::Type *getCanonical(const IR::Type *type);
    /// The width in bits of this type.  If the width is not
    /// well-defined this will report an error and return -1.
    /// max indicates whether we want the max width or min width.
    int widthBits(const IR::Type *type, const IR::Node *errorPosition, bool max) const;

    /// True is type occupies no storage.
    bool typeIsEmpty(const IR::Type *type) const;
};
}  // namespace P4

#endif /* FRONTENDS_P4_TYPEMAP_H_ */
