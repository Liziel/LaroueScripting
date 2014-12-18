#ifndef ASTScope_Resolution_h__
# define ASTScope_Resolution_h__
# ifndef INCLUDED_FROM_ASTNODE
#  error "this file should be inclued only in architecture/ASTnode.h"
# endif

class ScopeResolution : public AST::node{
public:
  static const parser::syntax::identifier syntax_identifier = parser::syntax::identifier::scope_resolution;

public:
  ScopeResolution(const std::string& t_file_line, const node::syntax& t_node_syntax)
    : AST::node(t_file_line, t_node_syntax){}
};

#endif