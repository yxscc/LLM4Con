// include/Query/SharedFieldKey.h
//
// Field-level aggregation key for cross-thread shared-object identification.
//
// Two LLVM Values hash to the same SharedFieldKey iff they refer to the
// same "conceptual storage location" at the C level: the same global
// variable, or the same field-offset inside the same struct type. This
// lets accesses coming through different SSA values (e.g., `this->field`
// from different entry points, where the `this` parameters are distinct
// Phasar abstract locations) collapse into a single shared object.
//
// Stack-allocated storage (anything rooted at an AllocaInst) is rejected
// by returning std::nullopt from fromValue(), because stack slots in
// different threads are not concurrently shared in general.
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <functional>

namespace llvm {
class Value;
class Module;
}

namespace query {

struct SharedFieldKey {
    enum class Kind {
        Global,        // global variable
        StructField,   // field inside some heap/struct object
        Unknown        // fallback: cannot classify but may still be valid
    };

    Kind        kind        = Kind::Unknown;
    // Canonical name of the root object:
    //   * Kind::Global      -> global variable name
    //   * Kind::StructField -> struct type name (e.g. "struct.nft_net") or
    //                          an identifier derived from the underlying
    //                          object when the type is anonymous
    //   * Kind::Unknown     -> best-effort descriptor (e.g. "param:foo#0")
    std::string type_name;
    // Byte offset inside the root object.
    //   * 0 or -1 for whole-object access (Kind::Global or Kind::Unknown)
    //   * >=0 field byte offset for Kind::StructField
    int64_t     field_offset = -1;

    // Returns std::nullopt when v is stack-allocated (alloca-rooted) and
    // therefore not a legitimate cross-thread sharing candidate.
    static std::optional<SharedFieldKey> fromValue(const llvm::Value* v,
                                                   const llvm::Module& M);

    bool operator==(const SharedFieldKey& o) const {
        return kind == o.kind &&
               type_name == o.type_name &&
               field_offset == o.field_offset;
    }
    bool operator!=(const SharedFieldKey& o) const { return !(*this == o); }
    bool operator<(const SharedFieldKey& o) const {
        if (kind != o.kind) return static_cast<int>(kind) < static_cast<int>(o.kind);
        if (type_name != o.type_name) return type_name < o.type_name;
        return field_offset < o.field_offset;
    }

    std::size_t hash() const;
    std::string toString() const;
};

struct SharedFieldKeyHasher {
    std::size_t operator()(const SharedFieldKey& k) const { return k.hash(); }
};

} // namespace query
