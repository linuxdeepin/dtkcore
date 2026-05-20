// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

// Private member accessor using the explicit template instantiation technique.
//
// C++ Standard [temp.explicit]/12 states:
// "The usual access checking rules do not apply to names used to
// specify explicit instantiation definitions."
//
// This allows passing pointers to private/protected data members and
// member functions as template arguments in explicit instantiations,
// bypassing normal access control — without modifying the class definition
// and without the UB caused by "#define private public".
//
// NOTE: These helper structs must be in the SAME namespace as the Tag structs
// (global namespace, since the macros expand at file scope). If they were in a
// sub-namespace, the friend definition would create a different function than
// the friend declaration in the Tag struct, causing undefined-reference errors.

#if defined(__GNUC__) && !defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif

template<typename Tag>
struct DtkCorePrivateAccessor
{
    using MemberPtr = typename Tag::MemberPtr;
    friend MemberPtr get(Tag) noexcept;
};

template<typename Tag, typename Tag::MemberPtr Ptr>
struct DtkCorePrivateAccessorImpl : DtkCorePrivateAccessor<Tag>
{
    friend typename Tag::MemberPtr get(Tag) noexcept { return Ptr; }
};

#if defined(__GNUC__) && !defined(__clang__)
#  pragma GCC diagnostic pop
#endif

#define D_DECLARE_PRIVATE_MEMBER(TagName, ClassName, Member, MemberType) \
    struct TagName { \
        using MemberPtr = MemberType ClassName::*; \
        friend MemberPtr get(TagName) noexcept; \
    }; \
    template struct DtkCorePrivateAccessorImpl<TagName, &ClassName::Member>

#define D_DECLARE_PRIVATE_METHOD(TagName, ClassName, MethodName, RetType, ...) \
    struct TagName { \
        using MemberPtr = RetType (ClassName::*)(__VA_ARGS__); \
        friend MemberPtr get(TagName) noexcept; \
    }; \
    template struct DtkCorePrivateAccessorImpl<TagName, &ClassName::MethodName>

#define D_DECLARE_PRIVATE_CONST_METHOD(TagName, ClassName, MethodName, RetType, ...) \
    struct TagName { \
        using MemberPtr = RetType (ClassName::*)(__VA_ARGS__) const; \
        friend MemberPtr get(TagName) noexcept; \
    }; \
    template struct DtkCorePrivateAccessorImpl<TagName, &ClassName::MethodName>

#define D_PRIVATE_MEMBER(obj, tag) ((obj).*get(tag))
#define D_PRIVATE_CALL(obj, tag, ...) ((obj).*get(tag))(__VA_ARGS__)

