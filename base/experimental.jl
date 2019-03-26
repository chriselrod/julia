# This file is a part of Julia. License is MIT: https://julialang.org/license

"""
    Experimental

Types, methods, or macros defined in this module are experimental and subject
to change and will not have deprecations. Caveat emptor.
"""
module Experimental

struct Const{T,N} <: DenseArray{T,N}
    a::Array{T,N}
end

Base.IndexStyle(::Type{<:Const}) = IndexLinear()
Base.size(C::Const) = size(C.a)
Base.axes(C::Const) = axes(C.a)
@eval Base.getindex(A::Const, i1::Int) =
    (Base.@_inline_meta; Core.const_arrayref($(Expr(:boundscheck)), A.a, i1))
@eval Base.getindex(A::Const, i1::Int, i2::Int, I::Int...) =
  (Base.@_inline_meta; Core.const_arrayref($(Expr(:boundscheck)), A.a, i1, i2, I...))

macro aliasscope(body)
    sym = gensym()
    esc(quote
        $(Expr(:aliasscope))
        $sym = $body
        $(Expr(:popaliasscope))
        $sym
    end)
end

end
