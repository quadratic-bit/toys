#pragma once
#include <string>
#include <typeinfo>
#include <vector>

#include "./trace/common.hpp"

using std::string;
using std::vector;

struct TypeIndex { enum Enum {
    Unknown,

    Double,
    Color,
    String,
    Vector3,

    Material,

    __COUNT
};};

template<typename T>
struct TypeToEnum { enum {
    value = TypeIndex::Unknown
};};

#define REGISTER_TYPE(CppType, EnumVal) \
  template<> struct TypeToEnum< CppType > { enum { value = EnumVal }; }


REGISTER_TYPE(Vector3, TypeIndex::Vector3);

struct Field {
    virtual ~Field() {}

    virtual string   name() const = 0;
    virtual unsigned type() const = 0;
    virtual void    *ptr()  const = 0;

    template<typename T>
    T &as() const {
        if (type() != TypeToEnum<T>::value) throw std::bad_cast();
        return *static_cast<T*>(ptr());
    }

    virtual string serialize() const = 0;
    virtual void   deserialize(const string &s) = 0;
};

template<typename Owner, typename T>
class FieldMember : public Field {
    Owner    *owner;
    T Owner::*member;
    string    name_;

public:
    FieldMember(Owner *o, T Owner::*m, string name)
        : owner(o), member(m), name_(name) {}

    string   name() const { return name_; }
    unsigned type() const { return TypeToEnum<T>::value; }
    void    *ptr()  const { return (void*)(&(owner->*member)); }

    string serialize() const {
        return stringify<T>(owner->*member);

    }
    void deserialize(const string &s) {
        owner->*member = parse_from_string<T>(s);
    }
};

// owns temporaries emitted during collectFields
struct FieldList {
    std::vector<Field*> vec;

    ~FieldList() {
        for (size_t i = 0; i < vec.size(); ++i) delete vec[i];
    }

    void push(Field *f) {
        vec.push_back(f);
    }

    size_t size() const {
        return vec.size();
    }

    Field *operator[](size_t i){
        return vec[i];
    }
};

// emitter
template<typename Owner>
struct Fields {
    Owner *o;
    FieldList &out;

    Fields(Owner *o_, FieldList &out_) : o(o_), out(out_) {}

    template<typename T>
    Fields &add(T Owner::*m, const string &name) {
        out.push(new FieldMember<Owner, T>(o, m, name));
        return *this;
    }
};

struct Reflectable {
    virtual ~Reflectable() {};

    virtual void collectFields(FieldList &out) = 0;

    Field *retrieveField(const string &name) {
        FieldList fields;
        collectFields(fields);
        for (size_t i = 0; i < fields.size(); ++i) {
            if (fields[i]->name() == name) return fields[i];
        }
        return NULL;
    }
};
